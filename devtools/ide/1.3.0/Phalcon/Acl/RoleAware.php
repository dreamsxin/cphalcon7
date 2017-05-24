<?php 

namespace Phalcon\Acl {

	/**
	 * Phalcon\Acl\RoleAware initializer
	 */
	
	interface RoleAware {

		/**
		 * Returns the role name
		 *
		 * @return string
		 */
		public function getRoleName();

	}
}
