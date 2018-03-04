<?php
	chdir('..');
	
	function isChroma($i, $x, $y) {
		$c = imagecolorat($i, $x, $y);
		if ((($c >>  8) & 0xFF) < 0xFF) return false;
		if ((($c >>  0) & 0xFF) > 0x7F) return false;
		if ((($c >> 16) & 0xFF) > 0x7F) return false;
		return true;
	}

	function doTransparent($i, $x, $y) {
		if (isChroma($i, $x, $y)) {
			//echo 'chroma!';
			imagefill($i, $x, $y, imagecolorallocatealpha($i, 0, 0, 0, 0x7f));
		}
	}	

	//if (false)
	{
		foreach (scandir($path = 'ORI/WV') as $f) { 
			list($bf) = explode('.', $f);
			$rs = "{$path}/{$f}";
			$rd = "VER/MP3/{$bf}.MP3";
			
			if (substr($f, 0, 1) == '.') continue;
			
			echo "{$f}...";
			if (!file_exists($rd)) system("UTIL\\lame.exe --preset 32 {$rs} {$rd}");
			echo "Ok\n";
		}
	}
	
	//if (false)
	{
		$i2 = imagecreatetruecolor(640, 480);
		$path = 'ORI/SG';
		foreach (scandir($path) as $f) { 
		//foreach (array('OMAKE_8.png', 'OMAKE_8A.png') as $f) {
		//foreach (array('WAKU_A1.png', 'WAKU_A1T.png') as $f) {
			list($bf) = explode('.', $f);
			$rs = "{$path}/{$f}";
			//$rd = "VER/G/640x480/{$bf}.G";
			$rd = "VER/G/640x480/{$bf}.JPG";
			
			if (substr($f, 0, 1) == '.') continue;

			echo "$f...\n";
			
			if (preg_match('/^\\d{2,2}$/i', $bf)) {
				continue;
				echo "$bf\n";
			}
			
			if (preg_match('/^\\w{1,1}$/i', $bf)) {
				continue;
				echo "$bf\n";
			}
			
			if (substr($bf, 0, 1) == 'B') {
				continue;
			}
			
			if ($bf == "WAKU_P") {
				continue;
			}
			
			//if (file_exists($rd)) continue;
			
			$i1 = imagecreatefrompng($rs);
			
			if (!imageistruecolor($i1)) {
				$ii = imagecreatetruecolor(imageSX($i1), imageSY($i1));
				imagecopy($ii, $i1, 0, 0, 0, 0, 640, 480);
				$i1 = $ii;
				//echo "error!\n";
			}
			
			doTransparent($i1, 0, 0);
			doTransparent($i1, 450, 265);
			doTransparent($i1, 60, 60);
			
			imagecopy($i2, $i1, 0, 0, 0, 0, 640, 480);
			
			$i3 = imagecreatetruecolor(imageSX($i1), imageSY($i1));
			imagecopy($i3, $i2, 0, 0, 0, 0, 640, 480);
			
			imagejpeg($i3, $rd, 100);
			//imagejpeg($i3, $rd, 85);
			//imagepng($i3, $rd);
			//exit;
		}
	}	
?>