<?php
	$params = array(
		'10'  => array(0, 34, 60, 100, 68, 53),
		/*
		'02A' => array(0, 0, 109, 101, 61, 47),
		'12'  => array(0, 0, 111, 47, 68, 54),
		'11'  => array(0, 0, 102, 54, 68, 51),
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
		*/
	);
	
	function toTrueColor($i) {
		$i2 = imagecreatetruecolor($w = imageSX($i), $h = imageSY($i));
		imagecopy($i2, $i, 0, 0, 0, 0, $w, $h);
		return $i2;
	}
	
	function alphai($i) {
		ImageSaveAlpha($i, true);
		imagealphablending($i, false);
		imagefilledrectangle($i, 0, 0, imageSX($i), imageSY($i), imagecolorallocatealpha($i, 0, 0, 0, 0x7f));
	}
	
	function applyMask($ic, $im, $bxa = 0) {
		ImageSaveAlpha($ic, true);
		imagealphablending($ic, false);

		$w = imageSX($ic); $h = imageSY($ic);
		for ($y = 0; $y < $h; $y++) {
			for ($x = 0; $x < $w; $x++) {
				$c = @imagecolorat($ic, $x, $y);
				$a1 = $c >> 24;
				$a2 = 0x7F - ((@imagecolorat($im, $x + $bxa, $y) & 0xFF) * 0x7F / 0xFF);
				$a = $a2;
				if ($a1 == 0x7F) $a = 0x7F;
				$c = ($c & ~0xFF000000) | ($a << 24);
				imagesetpixel($ic, $x, $y, $c);
			}
		}
	}
	
	foreach ($params as $k => $p) {
		list($rw, $bxa, $bx, $by, $bw, $bh) = $p;
		$col = toTrueColor(imageCreateFromPng("bmp/B{$k}_1A.png"));
		$msk = toTrueColor(imageCreateFromPng("bmp/B{$k}_0.png"));
		$i0 = imageCreatetruecolor(288, 376);
		
		alphai($i0);
		
		list($w, $h) = array(imageSX($col), imageSY($col));		
		
		for ($y = 0; $y < $h; $y++) {
			for ($x = 0; $x < $w; $x++) {
				$c = imagecolorat($col, $x, $y) & ~0xFF000000;
				$c |= (0x7F - ((imagecolorat($msk, $x, $y) & 0xFF) * 0x7F / 0xFF)) << 24;
				imagesetpixel($i0, $x + $bxa, $y, $c);
			}
		}
		
		@mkdir("fla/{$k}/", 0777, true);
		imagepng($i0, "fla/{$k}/1.png");
		
		$in = array();
		
		for ($n = 2; $n <= 6; $n++) {
			$coln = @imageCreateFromPng("bmp/B{$k}_{$n}A.png");
			if (!$coln) continue;
			$imgn = imageCreatetruecolor(288, 376);
			alphai($imgn);
			@imagecopy($imgn, $coln, $bx + $bxa, $by, $bx, $by, $bw, $bh);
			applyMask($imgn, $msk, -$bxa);
			imagepng($imgn, "fla/{$k}/{$n}.png");
		}
	}
	
	//system('php build.php');
?>