<?php

namespace Resultset;

class Robots extends \Phalcon\Mvc\Model
{
	public static function cloneResultMap($base, array $data, array $columnMap, int $dirtyState, $keepSnapshots = NULL, $sourceModel = NULL) {
		return new \ArrayObject($data);
	}
}
