<!DOCTYPE html>

<?php
if(isset($_POST['resetmap'])) 
{ 
  $date = date_create();
  $fdate = date_format($date, 'YmdHis');
  $curdir=getcwd();
  unlink($curdir.'/index.php');
  copy($curdir.'/index.template',$curdir.'/index.php');
  exec('chmod 664 index.php');
  header("refresh:3");
}
?>



<html>
  <head>
    <meta name="viewport" content="initial-scale=1.0, user-scalable=no">
    <meta charset="utf-8">
    <title>Hunter Tracker</title>
    <style>
		@media screen and (min-width: 840px){
			body {
				display: grid;
				grid-template-columns: 3fr 1fr;
				grid-gap: 1em;
			}
			
			/* Always set the map height explicitly to define the size of the div
			* element that contains the map. */
			#map {
				height: 93%;
				border: 1px solid black;
			}
			  
			/* Optional: Makes the sample page fill the window. */
			html, body {
				height: 99%;
			}
				
			#button {
				width: 100%;
				font-size: 20px;
				background-color: #ebebeb;
				border: 1px solid black;
				color: black;
				padding: 15px 32px;
				text-align: center;
				text-decoration: none;
				cursor: pointer;
				margin-top: 10px;
			}
			
			#right {
				height: 100%;
				background-color: #d1eaff;
				border: 1px solid black;
			}
			
			#previous {
				margin: 0;
				text-align: center;
			}
      input[type=submit] {
      width: 20em;  height: 4em;
      font-size:20px;
      background-color:#FDFFB6;
              }
      
		}
		
		@media screen and (max-width: 600px) {
			body {
				display: grid;
			}
			
			/* Always set the map height explicitly to define the size of the div
			* element that contains the map. */
			#map {
				height: 75vh;
				border: 1px solid black;
			}
			  
			/* Optional: Makes the sample page fill the window. */
			html, body {
				height: 99%;
			}
				
			#button {
				width: 100%;
				font-size: 20px;
				background-color: #ebebeb;
				border: 1px solid black;
				color: black;
				padding: 15px 32px;
				text-align: center;
				text-decoration: none;
				cursor: pointer;
				margin-top: 10px;
			}
			
			input[type=submit] {
        width: 100%;
        height: 4em;
        font-size:20px;
        background-color:#FDFFB6;
        
      }
			#previous {
				margin: 0;
				text-align: center;
			}
		}
    </style>
  </head>
  <body>
	<div id="left">
    <div id="map"></div>
    <script>
      function RefreshWindow()
      {
         window.location.reload(true);
      }

      // This example creates a 2-pixel-wide red polyline showing the path of
      // the first trans-Pacific flight between Oakland, CA, and Brisbane,
      // Australia which was made by Charles Kingsford Smith.

      function initMap() {

        var map = new google.maps.Map(document.getElementById('map'), {
          zoom: 17,
          // Set where you want your map to center to
          center: {lat: 00.00000,lng:-00.00000},
          mapTypeId: google.maps.MapTypeId.ROADMAP,

        });

        var flightPlanCoordinates = [

          //NEXT COORD
        ];
        var flightPath = new google.maps.Polyline({
          path: flightPlanCoordinates,
          geodesic: true,
          strokeColor: '#FF0000',
          strokeOpacity: 1.0,
          strokeWeight: 2
        });

        flightPath.setMap(map);
      // Set this to the same as your center location
      var currentPos = {lat: 00.00000,lng:-00.00000}
      var marker = new google.maps.Marker({
        position: currentPos,
        map: map,
        title: 'Last Seen'
			});


      }
      
  
    </script>

	<script async defer
	// You'll need an API key to use google maps. Put your key here.
	src="https://maps.googleapis.com/maps/api/js?key=<YOUR API KEY HERE>&callback=initMap">
	</script>

    <button id="button" onclick="return RefreshWindow();"/>Refresh</button>

	</div>

	
	<div id="right">
    <div id="previous"><div id="text">
    <p>
    <form method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
      <input type="submit" name="resetmap" value="Reset Map"><br>
    </form>

      <p><a href='index.php'>CURRENT TRACK</a></b>
      <p>Previous Tracks :</p><br> 
          <a href="track-01.php">Track-01</a>
      </div>
    </div>
	</div>
  </body>
</html>
