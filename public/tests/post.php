<?php

	if (!empty($_POST["var1"]) && !empty($_POST["var2"])):
		echo "CHECK OF VARS FROM POST : <br/>";
		echo "VAR1 = ".$_POST["var1"]."<br/>";
		echo "VAR2 = ".$_POST["var2"]."<br/>";
		return ;
	elseif (isset($_FILES["file"])):
		echo "FILE PUT INFORMATIONS : <br/>";
		echo "NAME = ".$_FILES["file"]["name"]."<br/>";
		echo "SIZE = ".$_FILES["file"]["size"]."<br/>";
		echo "TYPE = ".$_FILES["file"]["type"]."<br/>";
		return ;
	endif;
	echo "EMPTY VARS !";
?>