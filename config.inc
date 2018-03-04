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
define('TW_OAUTH_TOKEN', '2151724993-Y9ud9ZTCCF8XESMt9z6vXtPdQtIabQKQMH2qgCT');
define('TW_OAUTH_SECRET', 'IPuLHpzffufbNXEY4vb3vua67DLC1VdUZNPIeM3sRKmqe');
define('FB_TOKEN', '246063348785022|mUCXSG8ZR0uQ46tURVESg7Efpk8');
