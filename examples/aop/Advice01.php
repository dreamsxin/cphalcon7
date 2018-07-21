<?php

class MyServices
{
   public function doAdminStuff ()
   {
	  //some stuff only the admin should do
	  echo "Calling doAdminStuff".PHP_EOL;
   }

   public function doCount()
   {
	  return 1;
   }

   public function doHello($world = NULL)
   {
	  return 'Hello '.$world;
   }
}

class Advice {
	static public function forDoAdmin($obj) {
		var_dump($obj->getClassName());
		var_dump($obj->getMethodName());
		var_dump($obj->getArguments());
		if ((! isset($_SESSION['user_type'])) || ($_SESSION['user_type'] !== 'admin')) {
			throw new Exception('Sorry, you should be an admin to do this'.PHP_EOL);
		}
	}
	static public function forCount($obj) {
		return $obj->getReturnedValue() + 1;
	}
	static public function forHello($obj) {
		$obj->setArguments(['World!']);
		return $obj->process();
	}
}
Phalcon\Aop::addBefore('MyServices->doAdminStuff()', 'Advice::forDoAdmin');
Phalcon\Aop::addAfter('MyServices->doCount()', 'Advice::forCount');
Phalcon\Aop::addAround('MyServices->doHello()', 'Advice::forHello');

$services = new MyServices();
try {
   $services->doAdminStuff();
} catch (Exception $e) {
   echo $e->getMessage().PHP_EOL;
}

$_SESSION['user_type'] = 'admin';

$services->doAdminStuff();
echo $services->doCount().PHP_EOL;
echo $services->doHello().PHP_EOL;
