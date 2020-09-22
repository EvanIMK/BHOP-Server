<?php
require 'config.php';
require 'functions.php';
require 'steamid.php';

$connection = new mysqli(DB_HOST, DB_USER, DB_PASSWORD, DB_SCHEMA);
$connection->set_charset('utf8');

$style = 0;

if (isset($_REQUEST['style'])) {
    $style = $_REQUEST['style'];
}

$map = '';

if (isset($_REQUEST['map'])) {
    $map = $_REQUEST['map'];
}

$track = 0;

if (isset($_REQUEST['track'])) {
    $track = $_REQUEST['track'];
}

$rr = isset($_REQUEST['rr']);

$username = '';

if (isset($_REQUEST['username'])) {
    $username = $_REQUEST['username'];
}

$stype = 0;

if (isset($_REQUEST['stype'])) {
    $stype = $_REQUEST['stype'];
}

$top = isset($_REQUEST['top']);

if (API_KEY != false) {SteamID::SetSteamAPIKey(API_KEY);}
?>

<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="bhoptimer">

    <!-- favicon -->
    <link href="assets/icons/favicon.ico" rel="icon" type="image/x-icon" />

    <?php
    if (!$map) {
        echo '<title>'.HOMEPAGE_TITLE.'</title>';
    } else {
        echo '<title>'.removeworkshop($_GET['map']).'</title>';
    } ?>

    <!-- HTML5 shim and Respond.js for IE8 support of HTML5 elements and media queries -->
    <!--[if lt IE 9]>
      <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
      <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
    <![endif]-->
    <!-- let's hope maxcdn won't shut down ._. -->

    <!-- load jquery, pretty sure we need it for bootstrap -->
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>

    <!-- bootstrap itself -->
    <script async src="assets/js/bootstrap.min.js"></script>
    <script async src="assets/js/ie10-viewport-bug-workaround.js"></script>
    <script src="assets/js/searchbarplaceholder.js"></script>

    <!-- Bootstrap core CSS | can't late-load -->
    <link async rel="stylesheet" type="text/css" href="assets/css/bootstrap.min.css">

    <script>
    $(document).ready(function()
    {
        $("tr").hover(function()
        {
            if(!$(this).hasClass("lead") && $(this).attr('id') != "ignore")
            {
                $(this).addClass("mark");
            }
        },

        function()
        {
            if(!$(this).hasClass("lead") && $(this).attr('id') != "ignore")
            {
                $(this).removeClass("mark");
            }
        });
    });
    </script>
  </head>

  <body>
    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container">
        <div class="navbar-header">
          <a class="navbar-brand" href="index.php"><?php echo '<i class="fa fa-clock-o"></i> '.TOPLEFT_TITLE; ?></a>
        </div>
        <div id="navbar" class="navbar-collapse collapse">
            <ul class="nav navbar-nav">
                <li><a href="index.php?rr=1">Recent Records</a></li>
                <?php if (USES_RANKINGS == '1') { echo '<li><a href="index.php?top=1">Top Players</a></li>'; } ?>
            </ul>
            <form id="records" class="navbar-form navbar-right" method="GET">
                <div class="form-group">
                    <select name="style" class="form-control">
                        <?php
                        for ($i = 0; $i < count($styles); $i++) {
                            ?> <option value="<?php echo $i; ?>" <?php if ($i == DEFAULT_STYLE || $style == $i) {
                                echo 'selected="selected"';
                            } ?>><?php echo $styles[$i]; ?></option> <?php
                        }
                        ?>
                    </select>
                </div>
                <div class="form-group">
                    <select name="map" class="form-control" required>
                        <option value="" selected="selected">None</option>
                        <?php
                        $result = mysqli_query($connection, 'SELECT DISTINCT '.MYSQL_PREFIX.'map FROM maptiers ORDER BY map ASC;');

                        if ($result->num_rows > 0) {
                            while ($row = $result->fetch_assoc()) {
                                // $row["map"] - including workshop
                                // removeworkshop($row["map"]) - no workshop
                                ?> <option value="<?php echo $row['map']; ?>" <?php if ($row['map'] == $map) {
                                    echo 'selected="selected"';
                                } ?>><?php echo removeworkshop($row['map']); ?></option> <?php
                            }
                        }
                        ?>
                    </select>
                </div>
                <div class="form-group">
                    <select name="track" class="form-control">
                        <?php
                        for ($i = 0; $i < count($tracks); $i++) {
                            ?> <option value="<?php echo $i; ?>" <?php if ($i == DEFAULT_TRACK || $track == $i) {
                                echo 'selected="selected"';
                            } ?>><?php echo $tracks[$i]; ?></option> <?php
                        }
                        ?>
                    </select>
                </div>
                <button type="submit" class="btn btn-primary">Lookup</button>
          </form>
        </div>
      </div>
    </nav>

    <div class="container-fluid">
      <div class="row-fluid">
        <div class="col-md-10 col-md-offset-1">
          <div class="panel panel-default">
            <div class="panel-heading cam-heading center">
              <?php echo HOMEPAGE_TITLE; ?> <strong>-</strong> Record Database
            </div>
            <div class="panel-body table-responsive">
        <?php
        if (!isset($_REQUEST['map']) && !$rr && !isset($_REQUEST['username']) && !$top) {
            ?>
            <h1><?php echo HEADER_TITLE; ?></h1>
            <p>
                To view the records of any map, please select it using the menu at the top right of this page.<br />
                Otherwise, you can search for a player by their SteamID or display name using the field below.
            </p>
            <br />

            <form id="search" method="GET">
                <div class="form-group">
                    <div class="input-group">
                        <div class="input-group-btn">
                            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Search by... <span class="caret"></span></button>
                                    <ul class="dropdown-menu">
                                        <!-- this is commented out as it's kind of unnecessary
                                            <li onclick="steamidsearchtype(0)"><label class="btn btn-default form-control"><input class="srchhide" type="radio" name="stype" value="0" />Name</label></li> -->
                                        <li onclick="steamidsearchtype(1)"><label class="btn btn-default form-control"><input class="srchhide" type="radio" name="stype" value="1" />SteamID</label></li>
                                        <li onclick="steamidsearchtype(2)"><label class="btn btn-default form-control"><input class="srchhide" type="radio" name="stype" value="2" />SteamID3</label></li>
                                        <li onclick="steamidsearchtype(3)"><label class="btn btn-default form-control"><input class="srchhide" type="radio" name="stype" value="3" />SteamID64</label></li>
                                        <li onclick="steamidsearchtype(4)"><label class="btn btn-default form-control"><input class="srchhide" type="radio" name="stype" value="4" />Steamcommunity.com Profile URL</label></li>
                                    </ul>
                        </div>                                
                                <input type="text" name="username" class="form-control username-input" aria-label="..." placeholder="Name">
                        <div class="input-group-btn">
                            <button type="submit" class="btn btn-primary">Search</button>
                        </div>
                    </div><!-- /input-group -->
                </div><!-- /form-group -->
            </form>
                <br />
                <br />
            <p>
                You may click <a href="index.php?rr=1">Recent Records</a> to view the latest <?php echo RECORD_LIMIT_LATEST; ?> records<?php if (USES_RANKINGS == '1') {echo ', click <a href="index.php?top=1">Top Players</a> to view the leaderboard';}; if (SERVER_IP != '') {echo ' or click <a href="'.SERVER_IP.'">here</a> to join the server.';} ?>
            </p>
                <br />
                <br />                
                <br />
                <br />
            </div>
            <?php
        } else {
            $results = false;
            $stmt = false;

            if ($rr) { //recent records
                if (USES_RANKINGS == '0') {
                    $stmt = $connection->prepare('SELECT p.map, u.name, p.style, p.time, p.jumps, p.strafes, p.sync, u.auth, p.date, p.track FROM '.MYSQL_PREFIX.'playertimes p JOIN (SELECT MIN(time) time, map, style, track FROM '.MYSQL_PREFIX.'playertimes GROUP by map, style, track) t JOIN '.MYSQL_PREFIX.'users u ON p.time = t.time AND p.auth = u.auth AND p.map = t.map AND p.style = t.style AND p.track = t.track ORDER BY date DESC LIMIT '.RECORD_LIMIT_LATEST.' OFFSET 0;');
                } else {
                     $stmt = $connection->prepare('SELECT p.map, u.name, p.style, p.time, p.jumps, p.strafes, p.sync, u.auth, p.date, p.points, p.track FROM '.MYSQL_PREFIX.'playertimes p JOIN (SELECT MIN(time) time, map, style, track FROM '.MYSQL_PREFIX.'playertimes GROUP by map, style, track) t JOIN '.MYSQL_PREFIX.'users u ON p.time = t.time AND p.auth = u.auth AND p.map = t.map AND p.style = t.style AND p.track = t.track ORDER BY date DESC LIMIT '.RECORD_LIMIT_LATEST.' OFFSET 0;');
                }
                echo $connection->error;

                $stmt->execute();

                $stmt->store_result();

                $results = ($rows = $stmt->num_rows) > 0;

                if (USES_RANKINGS == '1') {
                    $stmt->bind_result($map, $name, $style, $time, $jumps, $strafes, $sync, $auth, $date, $points, $track);
                } else {
                    $stmt->bind_result($map, $name, $style, $time, $jumps, $strafes, $sync, $auth, $date, $track);
                }

                if ($rows > 0) {
                    $records = 0;

                    $first = true;

                    while ($row = $stmt->fetch()) {
                        if ($first) {
                            ?>
                            <table class="table table-striped table-hover">
                                <thead id="ignore">
                                    <th>Map</th>
                                    <th>Name</th>
                                    <th>Style / Track</th>
                                    <th>Time</th>
                                    <th>Jumps</th>
                                    <th>Strafes</th>
                                    <th>Sync</th>
                                    <th>Points</th>
                                    <th>SteamID</th>
                                    <th>Date <small>(YYYY-MM-DD)</small></th>
                                </thead>
                            <?php

                            $first = false;
                        } ?>

    					<tr>
                            <td><?php echo '<a href="index.php?style='.$style.'&map='.removeworkshop($map).'&track='.$track.'">'.removeworkshop($map).'</a>'; ?></td>
        					<td><?php echo '<a href="index.php?username='.$name.'">'.$name.'</a>'; ?></td>
        					<td><?php echo $styles[$style].' / '.trackname($track);?></td>
        					<td><?php echo formattoseconds($time); ?></td>
        					<td><?php echo $jumps; ?></td>
                            <td><?php echo $strafes; ?></td>
                            <td><?php echo number_format($sync, 2); ?>%</td>
                            <td><?php if (USES_RANKINGS == '1') {
                                echo number_format($points, 2);
                                }  
                                else {
                                    echo '---';
                                } ?>
                            </td>
                            <td><?php
                            $steamid = SteamID::Parse($auth, SteamID::FORMAT_S32);
                        echo '<a href="https://steamcommunity.com/profiles/'.$steamid->Format(SteamID::FORMAT_STEAMID64).'/" target="_blank">'.$auth.'</a>'; ?></td>

        					<td><?php if ($date[4] == '-') {
                            echo $date;
                        } else {
                            echo date('Y-m-d H:i:s', $date);
                        } ?></td>
                        </tr>
                        
                        <?php
                        if (++$records > RECORD_LIMIT_LATEST) {
                            break;
                        }
                    }
                }
            } elseif (!$username && !$top && !$stype) { //map dropdown

                if (USES_RANKINGS == '0') { 
                    $stmt = $connection->prepare('SELECT p.id, u.auth, u.name, p.time, p.jumps, p.strafes, p.sync, p.date FROM '.MYSQL_PREFIX.'playertimes p JOIN '.MYSQL_PREFIX.'users u ON p.auth = u.auth WHERE p.map = ? AND p.style = ? AND p.track = ? ORDER BY time ASC;'); 
                } else { 
                $stmt = $connection->prepare('SELECT pt.id, u.auth, u.name, pt.time, pt.jumps, pt.strafes, pt.sync, pt.date, pt.points FROM '.MYSQL_PREFIX.'playertimes pt JOIN '.MYSQL_PREFIX.'users u ON pt.auth = u.auth WHERE pt.map = ? AND pt.style = ? AND pt.track = ? ORDER BY time ASC;'); 
                } 
                $stmt->bind_param('sss', $map, $style, $track);
                $stmt->execute();

                $stmt->store_result();

                $results = ($rows = $stmt->num_rows) > 0;

                if (USES_RANKINGS == '1') {
                    $stmt->bind_result($id, $auth, $name, $time, $jumps, $strafes, $sync, $date, $points);
                } else {
                    $stmt->bind_result($id, $auth, $name, $time, $jumps, $strafes, $sync, $date);
                }

                if ($rows > 0) {
                    $first = true;

                    $rank = 1;

                    while ($row = $stmt->fetch()) {
                        if ($first) {
                            ?>
                            <p><span class="mark"><?php echo $styles[$style]; ?></span> Records (<?php echo number_format($rows); ?>) for <i><?php echo removeworkshop($map); ?></i> (<?php echo trackname($track); ?>):</p>

    						<table class="table table-striped table-hover">
    						<thead id="ignore"><th>Rank</th>
    						<th>Run ID</th>
    						<th>SteamID</th>
    						<th>Name</th>
    						<th>Time</th>
    						<th>Jumps</th>
                            <th>Strafes</th>
                            <th>Sync</th>
    						<th>Points</th>
                            <th>Date <small>(YYYY-MM-DD)</small></th></thead>

    						<?php

                            $first = false;
                        } ?>

                        <?php if ($rank == 1) {
                            ?>
                            <tr class="warning">
                            <?php
                        } else { 
                            ?>
                            <tr class="default">
                            <?php
                        } ?>    
                        <td>
                        <?php switch ($rank) {
                            case 1:
                            {
                                echo '<i class="fa fa-trophy" style="color:#E8C153"></i>';
                                break;
                            }

                            case 2:
                            {
                                echo '<i class="fa fa-trophy" style="color:#A8A8A8"></i>';
                                break;
                            }

                            case 3:
                            {
                                echo '<i class="fa fa-trophy" style="color:#965A38"></i>';
                                break;
                            }

                            default:
                            {
                                echo '#'.$rank;
                                break;
                            }
                        } ?></td>
    					<td><?php echo $id; ?></td>
    					<td><?php
                        $steamid = SteamID::Parse($auth, SteamID::FORMAT_S32);
                        echo '<a href="https://steamcommunity.com/profiles/'.$steamid->Format(SteamID::FORMAT_STEAMID64).'/" target="_blank">'.$auth.'</a>'; ?></td>
    					<td><?php echo '<a href="index.php?stype=2&username='.$auth.'">'.$name.'</a>'; ?></td>
    					<td><?php echo formattoseconds($time); ?></td>
    					<td><?php echo $jumps; ?></td>
                        <td><?php echo $strafes; ?></td>
                        <td><?php echo number_format($sync, 2); ?>%</td>
                        <td><?php if (USES_RANKINGS == '1') {
                            echo number_format($points, 2);
                        } else {
                            echo '---';
                        } ?></td>
                        <td><?php if ($date[4] == '-') {
                            echo $date;
                        } else {
                            echo date('Y-m-d H:i:s', $date);
                        } ?></td></tr>
    					<?php

                        if (++$rank > RECORD_LIMIT) {
                            break;
                        }
                    } ?> </table> <?php
                }
            } elseif ($top && (USES_RANKINGS == '1')) {
                $stmt = $connection->prepare('SELECT auth, name, lastlogin, points FROM '.MYSQL_PREFIX.'users ORDER BY points DESC LIMIT '.PLAYER_TOP_RANKING_LIMIT.' OFFSET 0;');
                $stmt->execute();
                $stmt->store_result();
                $results = ($rows = $stmt->num_rows) > 0;
                $stmt->bind_result($auth, $name, $lastlogin, $points);

                if ($rows > 0) {
                    $first = true;

                    $rank = 1;

                    while ($row = $stmt->fetch()) {
                        if ($first) {
                            ?>
                            <p><span class="mark">Top <?php echo PLAYER_TOP_RANKING_LIMIT; ?></span> Players</p>

                            <table class="table table-striped table-hover">
                            <thead id="ignore">
                            <th>Rank</th>
                            <th>SteamID</th>
                            <th>Name</th>
                            <th>Last Seen</th>
                            <th>Points</th>
                            </thead>

                            <?php

                            $first = false;
                        } ?>

                        <?php if ($rank == 1) {
                            ?>
                            <tr class="warning">
                            <?php
                        } else { 
                            ?>
                            <tr class="default">
                            <?php
                        } ?>    

                        <td>
                        <?php switch ($rank) {
                            case 1:
                            {
                                echo '<i class="fa fa-trophy" style="color:#E8C153"></i>';
                                break;
                            }

                            case 2:
                            {
                                echo '<i class="fa fa-trophy" style="color:#A8A8A8"></i>';
                                break;
                            }

                            case 3:
                            {
                                echo '<i class="fa fa-trophy" style="color:#965A38"></i>';
                                break;
                            }

                            default:
                            {
                                echo '#'.$rank;
                                break;
                            }
                        } ?></td>
                        <td><?php
                        $steamid = SteamID::Parse($auth, SteamID::FORMAT_S32);
                        echo '<a href="https://steamcommunity.com/profiles/'.$steamid->Format(SteamID::FORMAT_STEAMID64).'/" target="_blank">'.$auth.'</a>'; ?></td>
                        <td><?php echo '<a href="index.php?username='.$name.'">'.$name.'</a>'; ?></td>
                        <td><?php echo date('Y-m-d H:i:s', $lastlogin); ?></td>
                        <td><?php echo $points; ?></td>
                        </tr>
                        <?php

                        if (++$rank > PLAYER_TOP_RANKING_LIMIT) {
                            break;
                        }
                    } ?> </table> <?php
                }
            } elseif ($username) { //search
            
            $authtemp = '';
            $sid = '';

                if ($stype == '0') {
                    if (USES_RANKINGS == '0') {
                        $stmt = $connection->prepare('SELECT p.id, u.auth, u.name, p.map, p.time, p.jumps, p.strafes, p.sync, p.date, p.style, p.track FROM '.MYSQL_PREFIX.'playertimes p JOIN '.MYSQL_PREFIX.'users u ON p.auth = u.auth WHERE u.name = ? ORDER BY date ASC;');
                    } else {
                        $stmt = $connection->prepare('SELECT pt.id, u.auth, u.name, pt.map, pt.time, pt.jumps, pt.strafes, pt.sync, pt.date, pt.points, pt.style, pt.track FROM '.MYSQL_PREFIX.'playertimes pt JOIN '.MYSQL_PREFIX.'users u ON pt.auth = u.auth WHERE u.name = ? ORDER BY date ASC;');
                    }
                } elseif ($stype > '0') { //FORMAT_AUTO can go wrong pretty easily and has its own extension for Exception so that's used here, the blank message on the second try catch is intentional atm
                    if ($stype == '1') {
                        $authtemp = SteamID::Parse($username, SteamID::FORMAT_STEAMID32);
                    } elseif ($stype == '2') {
                        $authtemp = SteamID::Parse($username, SteamID::FORMAT_STEAMID3);
                    } elseif ($stype == '3') {
                        $authtemp = SteamID::Parse($username, SteamID::FORMAT_STEAMID64);
                    } elseif ($stype == '4') {
                        try {
                            $authtemp = SteamID::Parse($username, SteamID::FORMAT_AUTO, true);
                        } catch (SteamIDResolutionException $e) {
                            echo 'Caught exception: ', $e->getMessage(), '<br />';
                        }
                    }
                        try {
                          if ($authtemp == false) { //Parse function for 32/3/64 returns false on failure, AUTO does both
                                throw new Exception('Unable to parse SteamID, check your search term and try again');
                            }
                                $sid = $authtemp->Format(SteamID::FORMAT_STEAMID3);
                        } catch (Exception $e) {
                            echo 'Caught exception: ', $e->getMessage(), '<br />';
                        }

                    if (USES_RANKINGS == '0') {
                        $stmt = $connection->prepare('SELECT p.id, u.auth, u.name, p.map, p.time, p.jumps, p.strafes, p.sync, p.date, p.style, p.track FROM '.MYSQL_PREFIX.'playertimes p JOIN '.MYSQL_PREFIX.'users u ON p.auth = u.auth WHERE u.auth = ? ORDER BY date ASC;');
                    } else {
                        $stmt = $connection->prepare('SELECT pt.id, u.auth, u.name, pt.map, pt.time, pt.jumps, pt.strafes, pt.sync, pt.date, pt.points, pt.style, pt.track FROM '.MYSQL_PREFIX.'playertimes pt JOIN '.MYSQL_PREFIX.'users u ON pt.auth = u.auth WHERE u.auth = ? ORDER BY date ASC;');
                    }
                }  

                if ($stype > '0') {
                    $stmt->bind_param('s', $sid);
                } else {
                    $stmt->bind_param('s', $username);
                }

                $stmt->execute();

                $stmt->store_result();

                $results = ($rows = $stmt->num_rows) > 0;

                if (USES_RANKINGS == '1') {
                    $stmt->bind_result($id, $auth, $name, $map, $time, $jumps, $strafes, $sync, $date, $points, $style, $track);
                } else {
                    $stmt->bind_result($id, $auth, $name, $map, $time, $jumps, $strafes, $sync, $date, $style, $track);
                }

                if ($rows > 0) {
                    $first = true;

                    $rank = 1;

                    while ($row = $stmt->fetch()) {
                        if ($first) {
                            ?>
                            <p><span class="mark"><?php echo $name; ?></span> Records (<?php echo number_format($rows); ?>) </p>

                            <table class="table table-striped table-hover">
                            <thead id="ignore"><th>Run ID</th>
                            <th>SteamID</th>
                            <th>Name</th>
                            <th>Map</th>
                            <th>Style / Track</th>
                            <th>Time</th>
                            <th>Jumps</th>
                            <th>Strafes</th>
                            <th>Sync</th>
                            <th>Points</th>
                            <th>Date <small>(YYYY-MM-DD)</small></th></thead>

                            <?php

                            $first = false;
                        } ?>

                        <tr>
                        <td><?php echo $id; ?></td>
                        <td><?php
                        $steamid = SteamID::Parse($auth, SteamID::FORMAT_S32);
                        echo '<a href="https://steamcommunity.com/profiles/'.$steamid->Format(SteamID::FORMAT_STEAMID64).'/" target="_blank">'.$auth.'</a>'; ?></td>
                        <td><?php echo $name; ?></td>
                        <td><?php echo '<a href="index.php?style='.$style.'&map='.removeworkshop($map).'&track='.$track.'">'.removeworkshop($map).'</a>'; ?></td>
                        <td><?php echo $styles[$style].' / '.$tracks[$track]; ?></td>
                        <td><?php echo formattoseconds($time); ?></td>
                        <td><?php echo $jumps; ?></td>
                        <td><?php echo $strafes; ?></td>
                        <td><?php echo number_format($sync, 2); ?>%</td>
                        <td><?php if (USES_RANKINGS == '1') {
                            echo number_format($points, 2);
                        } else {
                            echo '---';
                        } ?></td>
                        <td><?php if ($date[4] == '-') {
                            echo $date;
                        } else {
                            echo date('Y-m-d H:i:s', $date);
                        } ?></td></tr>
                        <?php

                        if (++$rank > RECORD_LIMIT) {
                            break;
                        }
                    } ?> </table> <?php
                }
            } 
            if ($stmt != false) {
                $stmt->close();
            }

            if (!$results) { 
                echo '<h1>No results!</h1> <br />';
                if ($username == false) {
                echo '<p>Try another username, Steam names only cache when someone visits the gameserver so names may differ. Try searching by SteamID instead.</p>';
            }   else { 
                echo '<p>Try another map or style, there may be some records!</p>';}
            }
        }
        ?>
      </div>
    </div>
  </div>
</div>
</div>
</div>

</body>

  <!-- load those lately because it makes the page load faster -->
  <!-- IE10 viewport hack for Surface/desktop Windows 8 bug -->
  <link rel="stylesheet" href="assets/css/ie10-viewport-bug-workaround.css">

  <!-- Custom styles for this template -->
  <?php
  if (PAGE_STYLE == '0') {
      echo '<link rel="stylesheet" href="timer.css">';
  } else {
      echo '<link rel="stylesheet" href="timer-red.css">';
  }
  ?>

  <!-- font awesome -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.5.0/css/font-awesome.min.css">
</html>