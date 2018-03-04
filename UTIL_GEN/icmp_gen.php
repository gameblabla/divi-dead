<?php
	$params = array(
		'02A' => array(0, 0, 109, 101, 61, 47),
		'12'  => array(0, 0, 111, 47, 68, 54),
		'11'  => array(0, 0, 102, 54, 68, 51),
		'10'  => array(0, 34, 60, 100, 68, 53),
		'09A' => array(0, 0, 87, 59, 65, 54),
		'09'  => array(0, 0, 87, 57, 68, 56),
		'08'  => array(0, 0, 90, 74, 72, 56),
		'07'  => array(350, 0, 120, 144, 69, 50),
		'06'  => array(0, 0, 100, 104, 68, 58),
		'05'  => array(0, 0, 122, 106, 61, 48),
		'04'  => array(0, 0, 113, 87, 67, 47),
		'03'  => array(0, 0, 125, 99, 65, 52),
		'02'  => array(0, 0, 133, 92, 65, 60),
		'01'  => array(0, 0, 101, 105, 69, 51),
		'13'  => array(0, 0, 144, 65, 73, 50),
		'12A' => array(0, 0, 79, 51, 70, 54),
	);
	
	foreach ($params as $k => $p) {
		list($rw, $bxa, $bx, $by, $bw, $bh) = $p;
		$i = imageCreateFromPng("bmp/B{$k}_1A.png");
		if ($rw <= 0) $rw = imageSX($i) + 1;
		$is = imageCreatetruecolor($rw, imageSY($i) + $bh + 1);
		
		$c = imagecolorsforindex($i, imagecolorat($i, 0, 0));
		imagefilledrectangle($is, 0, 0, imageSX($is), imageSY($is), imagecolorallocate($is, $c['red'], $c['green'], $c['blue']));
		
		imagecopy($is, $i, $bxa, 0, 0, 0, imageSX($i), imageSY($i));

		$m_v = 0;
		$m_i = 0;
		for ($n = 0; $n < 255; $n++) {
			$v = array_sum(imagecolorsforindex($i, $n));
			if ($v > $m_v) {
				$m_v = $v;
				$m_i = $n;
			}
		}
		
		$c = imagecolorsforindex($i, $m_i);
		$c = imagecolorallocate($is, $c['red'], $c['green'], $c['blue']);
		//$c = imagecolorallocate($is, 0xFF, 0xFF, 0xFF);
		
		for ($y = 0; $y < $bh; $y++) {
			imagesetpixel($is, imageSX($is) - 1, $y + $by, $c);
		}

		for ($x = 0; $x < $bw; $x++) {
			imagesetpixel($is, $x + $bx + $bxa, imageSY($is) - 1, $c);
		}
		
		for ($n = 2; $n <= 6; $n++) {
			$ni = @imageCreateFromPng("bmp/B{$k}_{$n}A.png");
			@imagecopy($is, $ni, ($n - 2) * $bw, imageSY($is) - 1 - $bh, $bx, $by, $bw, $bh);
		}
		
		$imasktc = $imask = imageCreateFromPng("bmp/B{$k}_0.png");
		$imasktc = imagecreate(imageSX($imask), imageSY($imask));

		for ($n = 0; $n < 0x100; $n++) {
			imagecolorset($imasktc, $n, $n, $n, $n);
		}

		$c = imagecolorsforindex($imask, imagecolorat($imask, 0, 0));
		imagefilledrectangle($imasktc, 0, 0, imageSX($imasktc), imageSY($imasktc), imagecolorallocate($imasktc, $c['red'], $c['green'], $c['blue']));
		imagecopy($imasktc, $imask, $bxa, 0, 0, 0, imageSX($imask), imageSY($imask));
		//imagetruecolortopalette($imasktc, 0, 16);
		//imagetruecolortopalette($imasktc, 0, 256);
		
		
		imagepng($imasktc, "B{$k}_M.png", 9, PNG_NO_FILTER);
		
		//imagecreate
		
		imagepng($is, '32/32_' . $k . '.png', 9, PNG_NO_FILTER);

		imagetruecolortopalette($is, 0, 128);
		imagepng($is, "B{$k}_C.png", 9, PNG_NO_FILTER);
	}
	
	system('php icmp_build.php');
?>