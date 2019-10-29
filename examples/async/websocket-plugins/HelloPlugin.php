<?php

class HelloPlugin{
	static public function indexAction($data) {
		return 'Re'.$data;
	}
}