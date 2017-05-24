<?php 

namespace Phalcon\Acl {

	/**
	 * Phalcon\Acl\ResourceAware initializer
	 */
	
	interface ResourceAware {

		/**
		 * Returns the resource name
		 *
		 * @return string
		 */
		public function getResourceName();

	}
}
