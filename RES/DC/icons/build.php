<?php
	// 48x32
	
	$f = fopen('../../VMU.CHARS', 'wb');

	for ($z = 1; $z <= 13; $z++) {
		$i = imagecreatefrompng(sprintf('%02d.png', $z));
		$s = '';
		for ($y = 0; $y < 32; $y++) {
			for ($x = 0; $x < 48; $x += 8) {
				$v = 0;
				for ($n = 0; $n < 8; $n++) {
					//$v |= ((imagecolorat($i, $x + $n, $y) != 0) << (7 - $n));
					$v |= ((imagecolorat($i, 47 - ($x + $n), 31 - ($y)) == 0) << (7 - $n));
				}
				$s .= chr($v);
			}
		}
		fwrite($f, $s);
		//echo strlen($s) . "\n";
	}
?>