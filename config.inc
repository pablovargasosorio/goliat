<?php
// Location for the data retrieved from plugins. There will be created 2 subdirectories:
// - /dump (XML files for each tweet/post)
// - /stats (Some info about the target of the sonda)
define('_path_data', '/opt/goliat/data');
define('_path_queue', '/tmp/goliat');

// Location for the sondas config files
define('_path_sondas', './sondas_examples');

// Location for the main log output from the Goliat daemon
define('_logfile', '/var/log/goliat_collector.log');

// Maximum number of XML files to save into the data dump FIFO
define('_profile_limit', 2048);

// **** Valid tokens are REQUIRED to get data from Facebook and Twitter ****
define('TW_OAUTH_TOKEN', '');
define('TW_OAUTH_SECRET', '');
define('FB_TOKEN', '');
