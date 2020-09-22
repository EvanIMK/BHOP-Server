<?php

// ip address to the mysql server
define('DB_HOST', '');

// mysql username
define('DB_USER', '');

// mysql password
define('DB_PASSWORD', '');

// mysql database (schema) name
define('DB_SCHEMA', '');

// amount of records that can be displayed
define('RECORD_LIMIT', '100');

// the page's title as seen in the homepage
define('HOMEPAGE_TITLE', 'Bhop Stats');

// title for the top left side of the screen
define('TOPLEFT_TITLE', 'Bhop Stats');

// mysql table prefix, leave empty unless changed in the server
define('MYSQL_PREFIX', '');

// header title
define('HEADER_TITLE', 'Welcome!');

//link to join server, of the format steam://{ip}:{port}
define('SERVER_IP', '');

// page styling
define('PAGE_STYLE', '1'); // 0 - Default | 1 - Red/Black

// setup multi styles here, ordering must correspond with indexing in configs/shavit-styles.cfg
$styles = [
    'Normal', // 0
    'Sideways', // 1
    'W-Only', // 2
    'Scroll', // 3
    '400 Vel', // 4
    'Half-Sideways', // 5
    'D-Only', // 6
    'Low Gravity',
    'Slow Motion', //8 - see above
    'A-Only', //8 - see above 
    'Segmented', //8 - see above 
	'TAS',
	
];

//presently bhoptimer only officially supports one bonus track
//adding more bonus tracks is possible, though "you'd need to edit MAX_TRACKS in shavit.inc, recompile and add proper translations in shavit.phrases.txt" - shavit
$tracks = [
	'Main', // 0
	'Bonus', // 1
];

define('DEFAULT_STYLE', 0); // 0 - normal

define('DEFAULT_TRACK', 0); // 0 - main

// amount of records that can be displayed in 'latest records'
define('RECORD_LIMIT_LATEST', '25');

// amount of players displayed in top players list
define('PLAYER_TOP_RANKING_LIMIT', '50');

// uses rankings?
define('USES_RANKINGS', '1');

//steam api key for vanity url lookup, can be gotten from http://steamcommunity.com/dev/apikey. not necessary, but recommended
define('API_KEY', '');