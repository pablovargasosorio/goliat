<?php
include_once 'lib/twitteroauth.php';

class twitteruser
{
    private $sonda_id;
    private $account_id;
    private $tmp_file;
    private $bypass;
    
    private $plugin_code = 'twitteruser';
    
    function __construct($request)
    {
        $this->sonda_id      = $request['sonda_id'];
        $this->account_id    = $request['account_id'];
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
        $status = self::get_status();
        
        // Process comments
        $ratio  = self::insert_data($output);
        // Save profile info
        self::update_status($status);
        
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
        
        $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, TW_OAUTH_TOKEN, TW_OAUTH_SECRET);
        
        $opts   = array('user_id' => $this->account_id);
        $method = 'statuses/user_timeline';
        	
        $output = objectToArray($connection->get($method, $opts));

        if ($output['errors'][0]['code'] == 88)
        {
            Util::log(_logfile, "\n*******************************RATE LIMIT EXCEEDED, bye bye...*************************\n");
            exit;
        }
        elseif($output['errors'][0]['message']) {
            Util::log(_logfile, "TwitterUser plugin error: ".$output['errors'][0]['message']."\n");
            exit;
        }

        sleep(2);

        return $output;
    }
    
    /**
     * Get the results of a Twitter API query request
     *
     * @return Array $output
     */
    public function get_status()
    {
        $output = array();
    
        $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, TW_OAUTH_TOKEN, TW_OAUTH_SECRET);
    
        $opts   = array('user_id' => $this->account_id);
        $method = 'users/show';
    
         
        $output = objectToArray($connection->get($method, $opts));
         
        if ($output['errors'][0]['code'] == 88)
        {
            Util::log(_logfile, "\n*******************************RATE LIMIT EXCEEDED, bye bye...*************************\n");
            exit;
        }
        elseif($output['errors'][0]['message']) {
            Util::log(_logfile, "TwitterUser plugin error: ".$output['errors'][0]['message']."\n");
            exit;
        }
    
        sleep(2);
         
        return $output;
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
            	
            if (is_array($data))
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
        $pdir      = _path_data;
        $datadir   = _path_data.'/dump';
        
        if (!is_dir($pdir))
        {
            mkdir($pdir, 0777, true);
        }
        
        if (!is_dir($datadir))
        {
            mkdir($datadir, 0777, true);
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
            
            // Clean by profile limit
            $_files        = scandir($datadir);
            $current_count = count($_files);
            if ($current_count > _profile_limit + 2)
            {
                $ret = FALSE;
                foreach ($_files as $_file)
                {
                    if (preg_match("/^\d+\.[^\.]+\.[^\.]+\.(.+)\.xml$/i", $_file, $found) && !$ret)
                    {
                        unlink($datadir."/".$_file);
                        $current_count--;
                        if ($current_count <= _profile_limit + 2)
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
    
    /**
     *
     * @param unknown_type $stream
     */
    private function update_status($data)
    {
        $pdir      = _path_data;
        $statsdir  = _path_data.'/stats';
    
        if (!is_dir($pdir))
        {
            mkdir($pdir, 0777, true);
        }
    
        if (!is_dir($statsdir))
        {
            mkdir($statsdir, 0777, true);
        }
    
        $file = $statsdir.'/'.$this->sonda_id.'.dat';
    
        $data     = array_flip($data);
        $xml      = new SimpleXMLElement('<post/>');
        array_walk_recursive($data, array ($xml, 'addChild'));
        $xml_data = $xml->asXML();
    
        $f = fopen($file, 'w');
        fputs($f, $xml_data);
        fclose($f);
    }
    
    function update_rrd($total)
    {
        $pdir      = _path_data;
        $statsdir  = _path_data.'/stats';
        
        if (!is_dir($pdir))
        {
            mkdir($pdir, 0777, true);
        }
        
        if (!is_dir($statsdir))
        {
            mkdir($statsdir, 0777, true);
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
    }
    
    /**
     * 
     * @param Int $ratio
     */
    function set_next_request($r)
    {
        if ($r <= 0)
        {
            $offset = 43200;
        }
        else
        {
            $offset = ($r > 1) ? $r*90 : 300;
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
        
        if (count($_files) <= 2 && preg_match("/^".preg_quote(_path_queue, '/')."\/\d\d\d\d\d\d\d\d\d\d$/", $_datedir))
        {
        		rmdir($_datedir);
        }
        
        if (!is_dir(_path_queue.'/'.$next_hour))
        {
            mkdir(_path_queue.'/'.$next_hour);
        }
        
        if (!$this->bypass)
        {
            $newfile = _path_queue.'/'.$next_hour.'/'.$next_time.'_'.$this->plugin_code.'_'.$this->sonda_id;
            
            $f = fopen($newfile, "w");fclose($f);
        }
    }
}
