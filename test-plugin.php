<?php 
// Config variables
include 'config.inc';

// External libraries
//require 'HTTP/Request.php';

// General classes
include 'util.inc';

// Plugins
include 'plugins/twitter.inc';
include 'plugins/twitteruser.inc';
include 'plugins/facebook.inc';
include 'plugins/facebookuser.inc';

$plugin_code = $argv[1];
$query       = $argv[2];

if (($plugin_code != 'tw' && $plugin_code != 'fb' && $plugin_code != 'tu' && $plugin_code != 'fu') || $query == '')
{
    echo "Usage: ./test-plugin.php [tw|fb|tu|fu] [query]\n";
    exit;
}

$plugin  = Util::get_plugin_by_code($plugin_code);

if ($plugin == 'facebookuser' || $plugin == 'twitteruser') {
    $request['account_id'] = $query;
} else {
    $request['query'] = $query;
}

$plugin_object = new $plugin($request);

$output = $plugin_object->get_request();

print_r($output);
