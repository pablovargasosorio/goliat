<?php

// External functions
function objectToArray($d)
{
    if (is_object($d)) {
        $d = get_object_vars($d);
    }
    if (is_array($d)) {
        return array_map(__FUNCTION__, $d);
    }
    else {
        return $d;
    }
}

function array2XML($obj, $array)
{
    foreach ($array as $key => $value)
    {
        if(is_numeric($key))
            $key = 'item' . $key;

        if (is_array($value))
        {
            $node = $obj->addChild($key);
            array2XML($node, $value);
        }
        else
        {
            $obj->addChild($key, htmlspecialchars($value));
        }
    }
}

// Class methods
class Util
{
    function get_plugin_by_code($source)
    {
        if ($source == 'tw')
        {
            return 'twitter';
        }
        elseif ($source == 'tu')
        {
            return 'twitteruser';
        }
        elseif ($source == 'fu')
        {
            return 'facebookuser';
        }
        elseif ($source == 'fb')
        {
            return 'facebook';
        }
        elseif ($source == 'gp')
        {
            return 'googleplus';
        }
        elseif ($source == 'yt')
        {
            return 'youtube';
        }
        elseif ($source == 'in')
        {
            return 'instagram';
        }
        elseif ($source == 'gn')
        {
            return 'gnews';
        }
        elseif ($source == 'gb')
        {
            return 'gblogs';
        }
        elseif ($source == 'gf')
        {
            return 'gforums';
        }
        elseif ($source == 'me')
        {
            return 'meneame';
        }
        elseif ($source == 'fc')
        {
            return 'forocoches';
        }
        elseif ($source == 'bn')
        {
            return 'bnews';
        }
        elseif ($source == 'vi')
        {
            return 'vimeo';
        }
        elseif ($source == 'ws')
        {
            return 'website';
        }
    }
    
    function nombrencode ($nombre) {
        //$nombre = utf8_encode($nombre);
    
        $nombre = str_replace ("".utf8_encode("á")."","a",$nombre);
        $nombre = str_replace ("".utf8_encode("é")."","e",$nombre);
        $nombre = str_replace ("".utf8_encode("í")."","i",$nombre);
        $nombre = str_replace ("".utf8_encode("ó")."","o",$nombre);
        $nombre = str_replace ("".utf8_encode("ú")."","u",$nombre);
        $nombre = str_replace ("".utf8_encode("Á")."","A",$nombre);
        $nombre = str_replace ("".utf8_encode("É")."","E",$nombre);
        $nombre = str_replace ("".utf8_encode("Í")."","I",$nombre);
        $nombre = str_replace ("".utf8_encode("Ó")."","O",$nombre);
        $nombre = str_replace ("".utf8_encode("Ú")."","U",$nombre);
        $nombre = str_replace ("".utf8_encode("ñ")."","n",$nombre);
        $nombre = str_replace ("".utf8_encode("Ñ")."","N",$nombre);
    
        $nombre = str_replace ("".utf8_encode("à")."","a",$nombre);
        $nombre = str_replace ("".utf8_encode("è")."","e",$nombre);
        $nombre = str_replace ("".utf8_encode("ì")."","i",$nombre);
        $nombre = str_replace ("".utf8_encode("ò")."","o",$nombre);
        $nombre = str_replace ("".utf8_encode("ù")."","u",$nombre);
        $nombre = str_replace ("".utf8_encode("À")."","A",$nombre);
        $nombre = str_replace ("".utf8_encode("È")."","E",$nombre);
        $nombre = str_replace ("".utf8_encode("Ì")."","I",$nombre);
        $nombre = str_replace ("".utf8_encode("Ò")."","O",$nombre);
        $nombre = str_replace ("".utf8_encode("Ù")."","U",$nombre);
    
        $nombre = str_replace ("".utf8_encode("ñ")."","n",$nombre);
        $nombre = str_replace ("".utf8_encode("Ñ")."","N",$nombre);
        $nombre = str_replace ("".utf8_encode("ü")."","u",$nombre);
        $nombre = str_replace ("".utf8_encode("Ü")."","U",$nombre);
    
        return $nombre;
    }
    
    function clean_text($str)
    {
        $str = str_replace('&ntilde;', utf8_encode('ñ'), $str);
        $str = str_replace('&amp;ntilde;', utf8_encode('ñ'), $str);
        return html_entity_decode(strip_tags($str));
    }
    
    function log($logfile, $string)
    {
        // File mode
        if (file_exists($logfile))
        {
            $f = fopen($logfile, 'a');
            fputs($f, $string);
            fclose($f);
        }
        // Print mode
        else
        {
            echo $string;
        }
    }
    
    function get_ua()
    {
        $ua = array(
            'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9) AppleWebKit/537.71 (KHTML, like Gecko) Version/7.0 Safari/537.71',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.73.11 (KHTML, like Gecko) Version/7.0.1 Safari/537.73.11',
            'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0',
            'Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (iPhone; CPU iPhone OS 7_0_4 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11B554a Safari/9537.53',
            'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36',
            'Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko',
            'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (iPad; CPU OS 7_0_4 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11B554a Safari/9537.53',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:25.0) Gecko/20100101 Firefox/25.0',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:26.0) Gecko/20100101 Firefox/26.0',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36',
            'Mozilla/5.0 (Windows NT 6.1; rv:25.0) Gecko/20100101 Firefox/25.0',
            'Mozilla/5.0 (Windows NT 5.1; rv:25.0) Gecko/20100101 Firefox/25.0',
            'Mozilla/5.0 (Windows NT 5.1; rv:26.0) Gecko/20100101 Firefox/26.0',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36',
            'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:26.0) Gecko/20100101 Firefox/26.0',
            'Mozilla/5.0 (Windows NT 6.1; rv:26.0) Gecko/20100101 Firefox/26.0',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_5) AppleWebKit/537.71 (KHTML, like Gecko) Version/6.1 Safari/537.71',
            'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:25.0) Gecko/20100101 Firefox/25.0',
            'Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)',
            'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_6_8) AppleWebKit/534.59.10 (KHTML, like Gecko) Version/5.1.9 Safari/534.59.10',
            'Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)');
        
        return $ua[rand(0, count($ua) - 1)];
    }
    
}
?>