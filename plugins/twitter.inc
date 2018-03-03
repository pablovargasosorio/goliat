<?php
include 'lib/twitteroauth.php';

class twitter
{
    private $user_id;
    private $profile_id;
    private $profile_limit;
    private $sonda_id;
    private $query;
    private $query_not;
    private $exact;
    private $spa_force;
    private $tmp_file;
    private $bypass;
    
    private $plugin_code = 'twitter';
    
    function __construct($request)
    {
        $this->user_id       = $request['user_id'];
        $this->profile_id    = $request['profile_id'];
        $this->profile_limit = $request['profile_limit'];
        $this->sonda_id      = $request['sonda_id'];
        $this->query         = $request['query'];
        $this->query_not     = $request['query_not'];
        $this->exact         = $request['exact'];
        $this->spa_force     = $request['spa_force'];
        $this->tmp_file      = $request['tmp_file'];
        $this->bypass        = FALSE;
    }
    
    /**
     * Process the request to Twitter API
     * 
     * @param Array $request
     */
    public function process_request()
    {
        // Query
        $output = self::get_request();
        
        // Process comments
        $ratio  = self::insert_data($output);
        
        // Predict and set the next query
        self::set_next_request($ratio);
    }
    
    /**
     * Get the results of a Twitter API query request
     * 
     * @return Array $output
     */
    public function get_request()
    {
        $output = array();
        $out    = '';

        $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, TW_OAUTH_TOKEN, TW_OAUTH_SECRET);
        
        // Searching users
        if (preg_match('/^userquery:(.+)/', $this->query, $matches))
        {
            $opts   = array('q' => $matches[1], 'count' => 5);
            $method = 'users/search';
        }
        // Searching an user status
        elseif (preg_match("/^\d+$/", $this->query))
        {
            $opts   = array('user_id' => $this->query);
            $method = 'statuses/user_timeline';
            //$out    = 'statuses';
        }
        // Normal query
        else
        {
            $opts    = array('q' => urlencode(Util::nombrencode($this->query)), 'count' => 100);
            $method  = 'search/tweets';
            $out     = 'statuses';
        	}

        	$_output = objectToArray($connection->get($method, $opts));

        	if ($_output['errors'][0]['code'] == 88)
        	{
        	    Util::log(_logfile, "\n*******************************RATE LIMIT EXCEEDED, bye bye...*************************\n");
        	    exit;
        	}
        	elseif($_output['errors'][0]['message']) {
                Util::log(_logfile, "Twitter plugin error: ".$_output['errors'][0]['message']."\n");
                exit;
            }
        	
        	$output  = ($out != '') ? $_output[$out] : $_output;
        	
        	sleep(2);
        	
        	return $output;
    }
    
    /**
     *
     * @param unknown_type $comment
     *
     * @return string
     */
    public function comment_is_valid($comment)
    {
        if (($this->exact || !preg_match("/\s/", $this->query)) && !preg_match("/^\d+$/", $this->query))
        {
            // No exacto y con espacios (OR), y que no sea un usuario
            if (!preg_match("/(\s|\"|\(|\[|\#|^)".quotemeta($this->query)."(\s|\,|\.|\;|\:|\!|\?|\"|\'|\]|\)|$)/i", $comment))
            {
                return "No presente (exact = $exact):".$this->query." en \"$comment\"";
            }
        }
        elseif (!preg_match("/^\d+$/", $this->query))
        {
            if (!preg_match("/(\s|\"|\'|\(|\[|\#|^)(".str_replace(" ", "|", quotemeta($this->query)).")(\s|\,|\.|\;|\:|\!|\?|\"|\'|\]|\)|$)/i", $comment))
            {
                return "No presente (exact = ".$this->exact."):".$this->query." en \"$comment\"";
            }
        }
    
        if ($this->query_not != "" && preg_match("/(\s|\"|\(|\[|\#|^)(".str_replace(" ", "|", quotemeta($this->query_not)).")(\s|\,|\.|\;|\:|\!|\?|\"|\'|\]|\)|$)/i", $comment)) return "Not:".$this->query_not." en \"$comment\"";
    
        return '';
    }
    
    /**
     * 
     * @param unknown_type $stream
     */
    private function insert_data($stream)
    {
        
        if ($this->bypass)
        {
            return 0;
        }
        
        $total = 0;
        $time1 = 0;
        $time2 = 0;
        $count = 0;
        
        
        foreach ($stream as $data)
        {
            // Get ratio
            $time = strtotime($data['created_at']);
            
            if ($time1 == 0 || $time < $time1)
            {
                $time1 = $time;
            }
            
            if ($time2 < $time)
            {
                $time2 = $time;
            }
            
            $total++;
            	
            
            // Check if the comment is valid
            $valid = self::comment_is_valid($data['text']);
            	
            if (is_array($data) 
            && ($data['from_user'] != "" || $data['user']['screen_name'] != "") 
            && !preg_match("/RT /", $data['text']) 
            && $valid == "")
            {
                $data['created_at'] = date("Y-m-d H:i:s", strtotime($data['created_at']));
        
                if ($data['from_user'] == "" && $data['user']['screen_name'] != "")
                {
                    $data['from_user']         = $data['user']['screen_name'];
                    $data['from_user_name']    = $data['user']['name'];
                    $data['from_user_id']      = $data['user']['id'];
                    $data['profile_image_url'] = $data['user']['profile_image_url'];
                }
        
                if (preg_match("/(http:\/\/[^\s]+)/", $data['text'], $found))
                {
                    $data['link'] = $found[1];
                }
                
                // Insert into data FIFO
                $inserted = self::fifo_insert($data);

                if ($inserted) {
                    $count++;
                }
            }
        }
        
        Util::log(_logfile, "$count/$total comments\n");

        if ($count > 0) {
            self::update_rrd($count);
        }
        
        if ($total > 0 && $time2 - $time1 > 0)
        {
            $r = ($time2 - $time1) / $total;
        }
        else
        {
            $r = 0;
        }
        
        return $r;
    }
    
    /**
     * 
     * @param Array $data
     */
    private function fifo_insert($data)
    {
        $dump_uuid = md5($data['text']);
        $date      = $data['created_at'];
        $pdir      = _path_data.'/'.$this->user_id.'/'.$this->profile_id;
        $datadir   = _path_data.'/'.$this->user_id.'/'.$this->profile_id.'/dump';
        
        if (!is_dir($pdir))
        {
            mkdir($pdir, 0777, true);
            if (!chown($pdir, "apache")) Util::log(_logfile, "Error chown del dir: $pdir\n");
            if (!chgrp($pdir, "apache")) Util::log(_logfile, "Error chgrp del dir: $pdir\n");
        }
        
        if (!is_dir($datadir))
        {
            mkdir($datadir, 0777, true);
            if (!chown($datadir, "apache")) Util::log(_logfile, "Error chown del dir: $datadir\n");
            if (!chgrp($datadir, "apache")) Util::log(_logfile, "Error chgrp del dir: $datadir\n");
        }
        
        $datafile  = $datadir.'/'.date("YmdHis",strtotime($date)).'.'.$this->sonda_id.'.'.$this->plugin_code.'.'.$dump_uuid.'.xml';
        
        if (!file_exists($datafile) && file_exists(_path_sondas.'/'.$this->sonda_id))
        {
            preg_match_all("/\#[a-zA-Z0-9\-\_��������������]+/", $data['text'], $ht_matches);
            
            $data = array_flip($data);
            $xml = new SimpleXMLElement('<post/>');
        
            if (count($ht_matches[0]) > 0)
            {
                $xml->addChild("hashtags");
                foreach ($ht_matches[0] as $ht)
                {
                    $xml->hashtags->addChild("hashtag", $ht);
                }
            }
        
            array_walk_recursive($data, array ($xml, 'addChild'));
            
            $xml_data = $xml->asXML();
            
            $f = fopen($datafile,"w");
            fputs($f, $xml_data);
            fclose($f);
            
            if (!chown($datafile, "apache")) Util::log(_logfile, "Error chown: $datafile\n");
            if (!chgrp($datafile, "apache")) Util::log(_logfile, "Error chgrp: $datafile\n");
            
            // Clean by profile limit
            $_files        = scandir($datadir);
            $current_count = count($_files);
            if ($current_count > $this->profile_limit + 2)
            {
                $ret = FALSE;
                foreach ($_files as $_file)
                {
                    if (preg_match("/^\d+\.[a-zA-Z0-9]{32}\.[^\.]+\.(.+)\.xml$/i", $_file, $found) && !$ret)
                    {
                        unlink($datadir."/".$_file);
                        $current_count--;
                        if ($current_count <= $this->profile_limit + 2)
                        {
                            $ret = TRUE;
                        }
                    }
                }
            }

            return TRUE;
        } else {
            return FALSE;
        }
    }
    
    function update_rrd($total)
    {
        $pdir      = _path_data.'/'.$this->user_id.'/'.$this->profile_id;
        $statsdir  = _path_data.'/'.$this->user_id.'/'.$this->profile_id.'/stats';
        
        if (!is_dir($pdir))
        {
            mkdir($pdir, 0777, true);
            if (!chown($pdir, "apache")) Util::log(_logfile, "Error chown del dir: $pdir\n");
            if (!chgrp($pdir, "apache")) Util::log(_logfile, "Error chgrp del dir: $pdir\n");
        }
        
        if (!is_dir($statsdir))
        {
            mkdir($statsdir, 0777, true);
            if (!chown($statsdir, "apache")) Util::log(_logfile, "Error chown del dir: $statsdir\n");
            if (!chgrp($statsdir, "apache")) Util::log(_logfile, "Error chgrp del dir: $statsdir\n");
        }
        
        $file = $statsdir.'/'.$this->sonda_id.'.rrd';
        
        $_output = (file_exists($file)) ? file($file) : array();
        $data    = array();
        
        foreach ($_output as $_line)
        {
            $_fields = explode(':', $_line);
            
            $data[$_fields[0]] = intval($_fields[1]);
        }
        
        
        // Year
        $_key_year = 'Y_'.date('Y');
        
        $data[$_key_year] += $total;
        
        // Month
        $m = intval(date('m'));
        
        $next = ($m == 12) ? 1 : $m + 1;
        
        for ($i = 1; $i <= 12; $i++)
        {
            $_key_month = 'm_'.$i;
            
            if ($i == $next)
            {
                $_val = 0;
            }
            elseif ($i == $m)
            {
                $_val = $data[$_key_month] + $total;
            }
            else
            {
                $_val = ($data[$_key_month] > 0) ? $data[$_key_month] : 0;
            }
            
            $data[$_key_month] = $_val;
        }
        
        // Day
        $d = intval(date('d'));
        
        $next    = array();
        $next[0] = intval(date('d', time() + 86400));
        
        if ($next[0] < $d)
        {
            for ($k = $d + 1; $k <= 31; $k++)
            {
                $next[] = $k;
            }
        }
        
        for ($i = 1; $i <= 31; $i++)
        {
            $_key_day = 'd_'.$i;
            
            if (in_array($i, $next))
            {
                $_val = 0;
            }
            elseif ($i == $d)
            {
                $_val = $data[$_key_day] + $total;
            }
            else
            {
                $_val = ($data[$_key_day] > 0) ? $data[$_key_day] : 0;
            }
            
            $data[$_key_day] = $_val;
        }
        
        $f = fopen($file, 'w');
        
        foreach ($data as $k => $v)
        {
            fputs($f, "$k:$v\n");
        }
        
        fclose($f);
        
        if (!chown($file, "apache")) Util::log(_logfile, "Error chown: $file\n");
        if (!chgrp($file, "apache")) Util::log(_logfile, "Error chgrp: $file\n");
    }
    
    /**
     * 
     * @param Int $ratio
     */
    function set_next_request($r)
    {
        $activity = database::get_activity($this->user_id);
        
        if ($r <= 0)
        {
            $offset = 43200;
        }
        else
        {
            $offset = ($r > 1) ? $r*90 : 300;
        }
        
        if (time() - strtotime($activity) < 60)
        {
            if ($r < 3600)
            {
                $offset = 30;
            }
        }
        
        if ($offset > 172800)
        {
            $offset = 172800;
        }
        
        $future_time = time() + $offset;
        $next_time   = date("YmdHis", $future_time);
        $next_hour   = date("YmdH",   $future_time);
        
        unlink($this->tmp_file);
        
		$_datedir = dirname($this->tmp_file);
		$_files   = scandir($_datedir);
        
        if (count($_files) <= 2 && preg_match("/^".preg_quote(_path_sondas, '/')."\/\d\d\d\d\d\d\d\d\d\d$/", $_datedir))
        {
        		rmdir($_datedir);
        }
        
        if (!is_dir(_path_sondas.'/'.$next_hour))
        {
            mkdir(_path_sondas.'/'.$next_hour);
            if (!chown(_path_sondas.'/'.$next_hour, "apache")) Util::log(_logfile, "Error chown del dir: "._path_sondas."/$next_hour\n");
            if (!chgrp(_path_sondas.'/'.$next_hour, "apache")) Util::log(_logfile, "Error chgrp del dir: "._path_sondas."/$next_hour\n");
        }
        
        if (!$this->bypass)
        {
            $newfile = _path_sondas.'/'.$next_hour.'/'.$next_time.'_'.$this->plugin_code.'_'.$this->sonda_id;
            
            $f = fopen($newfile, "w");fclose($f);
            if (!chown($newfile, "apache")) Util::log(_logfile, "Error chown del datefile: $newfile\n");
            if (!chgrp($newfile, "apache")) Util::log(_logfile, "Error chgrp del datefile: $newfile\n");
        }
    }
}
