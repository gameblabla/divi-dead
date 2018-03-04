<?php
	$path_to_abs = '../../ACME';
	$path_to_abs = 'C:/projects/2tales-tra.com/dividead/flash/ABS/EN';

	preg_match_all('@#id(\\d+)\\s+{(.*)}@Umsi', file_get_contents('map.html'), $res);
	$len = sizeof($res[0]);
	
	$rects = array();
	
	for ($n = 0; $n < $len; $n++) {
		$id = $res[1][$n];
		$data = $res[2][$n];
		preg_match_all('@(\\w+):([^;]+);@Umsi', $data, $res2);
		$len2 = sizeof($res2[0]);
		
		$size = array(0, 0, 0, 0);
		
		for ($m = 0; $m < $len2; $m++) {
			$key = $res2[1][$m];
			$value = $res2[2][$m];
			switch ($key) {
				case 'left'  : $size[0] = (int)$value; break;
				case 'top'   : $size[1] = (int)$value; break;
				case 'width' : $size[2] = (int)$value; break;
				case 'height': $size[3] = (int)$value; break;
			}
			//echo "$key, $value\n";
		}
		
		$rect = array(
			$size[0],
			$size[1],
			$size[0] + $size[2],
			$size[1] + $size[3],			
		);
		
		$rects[$id] = $rect;
	}
	
	ksort($rects);
	foreach ($rects as $id => $rect) {
		echo "$id, $rect[0], $rect[1], $rect[2], $rect[3]\n";
		//echo "$rect[0], $rect[1], $rect[2], $rect[3]\n";
	}
	
	foreach (scandir($path = $path_to_abs) as $file) {
		$rfile = "$path/$file";
		if (!preg_match('@MAP\\d_\\d+.ABS@Umsi', $file)) continue;
		echo "$file...";
		$pos = 0;
		$data = '';
		foreach (file($rfile) as $line) {
			if (preg_match('@MAP_OPTION_@Umsi', $line)) {
				$pos = 0;
			} else if (preg_match('@MAP_OPTION@Umsi', $line)) {
				$values = explode(',', $line);
				$rect = $rects[$pos++];
				$values[1] = ' ' . (int)$rect[0];
				$values[2] = ' ' . (int)$rect[1];
				$values[3] = ' ' . (int)$rect[2];
				$values[4] = ' ' . (int)$rect[3] . "\n";
				$line = implode(',', $values);
				//echo "$line\n";
			}
			$data .= $line;
		}
		file_put_contents($rfile, $data);
		echo "Patched\n";
	}
	
	/*
 MAP_IMAGES "M_A1", "M_A2"
 MAP_OPTION_RESET
 MAP_OPTION label_1, 114, 42, 167, 78
 MAP_OPTION label_2, 182, 117, 237, 135
 MAP_OPTION label_3, 108, 147, 137, 183
 MAP_OPTION label_4, 75, 187, 98, 232
 MAP_OPTION label_5, 237, 136, 297, 155
 MAP_OPTION label_6, 258, 171, 297, 187
 MAP_OPTION label_7, 144, 227, 223, 249
 MAP_OPTION label_8, 275, 202, 321, 221
 MAP_OPTION label_9, 334, 202, 363, 220
 MAP_OPTION label_10, 332, 222, 350, 269
 MAP_OPTION label_11, 305, 279, 343, 315
 MAP_OPTION_SHOW	
	*/
?>