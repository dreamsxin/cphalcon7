<?php

namespace Boutique;

class Robots extends \Phalcon\Mvc\Model
{

	/**
	 * @Primary
	 * @Identity
	 * @Column(type="integer", size=32, bytes=4, scale=0, nullable=false)
	 */
	public $id;

	/**
	 * @Column(type="string", size=70, scale=0, nullable=false, column="name")
	 */
	public $robotName;

	/**
	 * @Column(type="string", size=32, scale=0, nullable=false)
	 */
	public $type;

	/**
	 * @Column(type="integer", size=32, bytes=4, scale=0, nullable=false)
	 */
	public $year;

}
