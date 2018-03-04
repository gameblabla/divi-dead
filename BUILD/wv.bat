<?php
	//oggenc2 -o WV2\AKI0001.OGG  --resample=22050 -q 1 WV\AKI0001.WAV
	
	foreach (scandir('WV') as $f) { $rf = "WV/{$f}";
		if (substr($f, 0, 1) == '.') continue;
		system("oggenc2 -o WV2/{$f}.OGG -q 1 WV/{$f}");
	}
?>