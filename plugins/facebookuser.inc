<?php
class facebookuser
{
    private $sonda_id;
    private $account_id;
    private $tmp_file;
    private $bypass;
    
    private $plugin_code = 'facebookuser';
    
    function __construct($request)
    {
        $this->sonda_id      = $request['sonda_id'];
        $this->account_id    = $request['account_id'];
        $this->allowed       = $request['allowed']; // TODO
        $this->fb_token      = $request['fb_token']; // TODO
        $this->tmp_file      = $request['tmp_file'];
        $this->bypass        = FALSE;
    }
    
    /**
     * Process the request to Facebook API
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
     * Get the results of a Facebook API query request
     *
     * @return Array $output
     */
    public function get_request()
    {
        $url = "https://graph.facebook.com/".$this->account_id."/feed?access_token=".FB_TOKEN."&fields=id,message,created_time,from,type,link,description,name,caption";
        
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        $result = curl_exec($ch);
        curl_close($ch);
        $output = json_decode($result, true);
        
        if ($output['error']['message'] != '')
        {
            Util::log(_logfile, 'Facebook plugin ERROR (token '.FB_TOKEN.'): '.$output['error']['message']."\n");
        }
        
        return $output['data'];
    }
    
    /**
     * Get the results of a Facebook API query request
     *
     * @return Array $output
     */
    public function get_status()
    {
        if ($this->allowed && $this->fb_token != '')
        {
            // TODO Private token for full account access
            $access_token = $this->fb_token;
            $endpoint = 'me';
            $params = '&fields=name,email,about,location';
        }
        else
        {
            $access_token = FB_TOKEN;
            $endpoint = $this->account_id;
            $params = '';
        }
        
        $url = "https://graph.facebook.com/".$endpoint."?access_token=".$access_token.$params;
        
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        $result = curl_exec($ch);
        curl_close($ch);
        $output = json_decode($result, true);
        
        if ($output['error']['message'] != '')
        {
            Util::log(_logfile, 'Facebook plugin ERROR (token '.$access_token.'): '.$output['error']['message']."\n");
        }
        
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
            $time = strtotime($data['created_time']);
            
            if ($time1 == 0 || $time < $time1)
            {
                $time1 = $time;
            }
            
            if ($time2 < $time)
            {
                $time2 = $time;
            }
            
            $total++;
                
            
            // Select best post message (message || description || name || caption)
            $title = "";
            if ($data['message'] != "")
            {
                $title = $data['message'];
            }
            elseif ($data['description'] != "")
            {
                $title = $data['description'];
            }
            elseif ($data['name'] != "")
            {
                $title = $data['name'];
            }
            elseif ($data['caption'] != "")
            {
                $title = $data['caption'];
            }
                
            if ($title != "")
            {
                $title = preg_replace("/\x{201c}/u", "\"", $title);
                $title = preg_replace("/\x{201d}/u", "\"", $title);
                
                // Cut the too long texts
                if (strlen($title) > 1500)
                {
                    $title = substr($title, 0, strpos(wordwrap($title, 1500), "\n"))." [...]";
                }

                $row = array(
                    "from_user"         => $data['from']['name'],
                    "from_user_id"      => $data['from']['id'],
                    "profile_image_url" => "https://graph.facebook.com/".$data['from']['id']."/picture",
                    "text"              => $title,
                    "created_at"        => date("Y-m-d H:i:s",strtotime($data['created_time'])),
                    "type"              => $data['type'],
                    "link"              => $data['link'],
                    "id"                => $data['id']
                );
                
                // Insert into data FIFO
                $inserted = self::fifo_insert($row);

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
        
        $data['allowed'] = $this->allowed;
        
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
            $data     = array_flip($data);
            $xml      = new SimpleXMLElement('<post/>');
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
