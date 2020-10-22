<?php

class IndexController extends \Phalcon\Mvc\Controller {

	public function indexAction() {
		return 'Hello world';
	}

	public function viewAction() {
		echo 'view -';
	}
}