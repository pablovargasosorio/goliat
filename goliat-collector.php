#! /usr/bin/php -q
<?php
ini_set('memory_limit', '1024M');
error_reporting(E_ERROR | E_PARSE);
ini_set('display_errors', 'On');
date_default_timezone_set("Europe/Madrid");

// Configuration variables
include 'config.inc';

// External libraries
//require 'HTTP/Request.php';

// General classes
include 'util.inc';

// Plugins
include 'plugins/twitter.inc';
include 'plugins/twitteruser.inc';
include 'plugins/facebookuser.inc';

// Inic
$f = fopen(_logfile, 'w');
fputs($f, "++++++++ GOLIAT COLLECTOR STARTED (".date("d/m/Y H:i:s").") ++++++++\n\n");
fclose($f);

// Check correct configuration
if (TW_OAUTH_TOKEN == '' || TW_OAUTH_SECRET == '' || FB_TOKEN == '') {
    fputs($f, "Please set the access tokens into the config.inc file\n");
    echo "Please fill the access tokens into the config.inc file\n";
    exit;
}

// Clean previous execution queue
$datedirs = scandir(_path_queue);
foreach ($datedirs as $datedir)
{
    if (preg_match("/\d\d\d\d\d\d\d\d\d\d/", $datedir))
    {
        $queue_files = scandir(_path_queue . '/' . $datedir);
        foreach ($queue_files as $file) if (preg_match("/^\d+\_[^\_]+\_.+$/i", $file)) {
            unlink(_path_queue."/$datedir/$file");
        }
        rmdir(_path_queue."/$datedir");
    }
}

$sonda_files = scandir(_path_sondas);
foreach ($sonda_files as $sonda_file) if ($sonda_file != '.' && $sonda_file != '..')
{
    if (preg_match("/[\_\.]/", $sonda_file)) {
        echo "Invalid characters in sonda file name '$sonda_file'. Please remove underscore and dot characters!\n";
        exit;
    }
    init_sonda(_path_sondas . '/' . $sonda_file);
}

while (TRUE)
{
    $elected  = FALSE;
    $what     = '';
    $datedirs = scandir(_path_queue);
    $sonda    = array();
    $request  = array();

    foreach ($datedirs as $datedir)
    {
        if (preg_match("/\d\d\d\d\d\d\d\d\d\d/", $datedir) && !$elected)
        {
            $files = scandir(_path_queue.'/'.$datedir);
            foreach ($files as $file)
            {
                if (preg_match("/^\d+\_[^\_]+\_.+$/i", $file) && !$elected)
                {
                    $fields   = explode('_', trim($file));
                    $filedate = $fields[0];
                    $plugin   = $fields[1];
                    $sonda_id = $fields[2];
                
                    if (file_exists(_path_sondas.'/'.$sonda_id))
                    {
                        $sonda_content = file(_path_sondas.'/'.$sonda_id);
                        if (count($sonda_content) < 1)
                        {
                            Util::log(_logfile, "Error loading sonda: "._path_sondas."/$sonda_id\n");
                        }
                        else
                        {
                            $request['sonda_id'] = $sonda_id;
                            $request['time']     = $filedate;
                            $request['tmp_file'] = _path_queue.'/'.$datedir.'/'.$file;
                            $request['profile_limit'] = 1000;
                            $request['exact'] = FALSE;
                            $request['allowed'] = FALSE;
                            foreach($sonda_content as $sonda_property)
                            {
                                $fields = explode('=', $sonda_property);
                                $request[$fields[0]] = trim($fields[1]);
                                if ($fields[0] == 'exact' && preg_match("/true/i", $fields[1])) {
                                  $request['exact'] = TRUE;
                                }
                                if ($fields[0] == 'allowed' && preg_match("/true/i", $fields[1])) {
                                    $request['allowed'] = TRUE;
                                }
                            }
                            $elected = TRUE;
                        }
                    }
                    else
                    {
                        unlink(_path_queue."/$datedir/$file");
                        $_files = scandir(_path_queue."/$datedir");
                        
                        if (count($_files) <= 2)
                        {
                            Util::log(_logfile, "Removing "._path_queue."/$datedir\n");
                            rmdir(_path_queue."/$datedir");
                        }
                    }
                }
            }
        }
    }

    if (date("YmdHis") >= $request['time'])
    {
        if (class_exists($plugin))
        {
            // QUERY TO THE INTERNET
            Util::log(_logfile, "Executing plugin -> ".ucfirst($plugin)."(".$request['sonda_id'].")\n");
            $plugin_object = new $plugin($request);
            $plugin_object->process_request();
        }
        else
        {
            Util::log(_logfile, "ERROR: Plugin '$plugin' not found\n");
            exit;
        }
    }
    else
    {
        // WAIT
        Util::log(_logfile, '.');
        sleep(2);
    }
}

function init_sonda($file) {
    $sonda = array();
    $sonda_content = file($file);
    $file_fields = explode('/', $file);
    $sonda['sonda_id'] = $file_fields[count($file_fields) -1];
    foreach($sonda_content as $sonda_property)
    {
        $fields = explode('=', $sonda_property);
        $sonda[$fields[0]] = trim($fields[1]);
    }
    
    if ($sonda['plugin'] != 'twitter' && $sonda['plugin'] != 'twitteruser' && $sonda['plugin'] != 'facebookuser') {
        Util::log(_logfile, "Error loading sonda: $file\n");
        return array();
    }
    
    $past_time = time() - 1000;
    $next_time   = date("YmdHis", $past_time);
    $next_hour   = date("YmdH",   $past_time);
    if (!is_dir(_path_queue.'/'.$next_hour))
    {
        if (!mkdir(_path_queue.'/'.$next_hour)) {
            Util::log(_logfile, "Error creating temporary file in: "._path_queue."\n");
            echo "Error creating temporary file in: "._path_queue."\n";
            exit;
        }
    }
    $newfile = _path_queue.'/'.$next_hour.'/'.$next_time.'_'.$sonda['plugin'].'_'.$sonda['sonda_id'];
    $f = fopen($newfile, "w");fclose($f);
}
?>
