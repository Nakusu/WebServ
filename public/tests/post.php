<?php

	if (!empty($_POST["var1"]) && !empty($_POST["var2"])):
		echo "CHECK OF VARS FROM POST : <br/>";
		echo "VAR1 = ".$_POST["var1"]."<br/>";
		echo "VAR2 = ".$_POST["var2"]."<br/>";
		return ;
	endif;
	echo "EMPTY VARS !";
?>