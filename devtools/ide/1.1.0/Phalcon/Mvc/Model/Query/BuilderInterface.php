<?php 

namespace Phalcon\Mvc\Model\Query {

	/**
	 * Phalcon\Mvc\Model\Query\BuilderInterface initializer
	 */
	
	interface BuilderInterface {

		/**
		 * Gets the type of PHQL queries
		 *
		 * @return int
		 */
		public function getType();


		/**
		 * Compile the PHQL query
		 *
		 * @return \Phalcon\Mvc\Model\QueryInterface
		 */
		public function compile();


		/**
		 * Returns a PHQL statement built based on the builder parameters
		 *
		 * @return string
		 */
		public function getPhql();


		/**
		 * Returns the query built
		 *
		 * @return \Phalcon\Mvc\Model\QueryInterface
		 */
		public function getQuery();

	}
}
