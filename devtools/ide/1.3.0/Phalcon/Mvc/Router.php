<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\Router
	 *
	 * <p>Phalcon\Mvc\Router is the standard framework router. Routing is the
	 * process of taking a URI endpoint (that part of the URI which comes after the base URL) and
	 * decomposing it into parameters to determine which module, controller, and
	 * action of that controller should receive the request</p>
	 *
	 *<code>
	 *
	 *	$router = new Phalcon\Mvc\Router();
	 *
	 *  $router->add(
	 *		"/documentation/{chapter}/{name}.{type:[a-z]+}",
	 *		array(
	 *			"controller" => "documentation",
	 *			"action"     => "show"
	 *		)
	 *	);
	 *
	 *	$router->handle();
	 *
	 *	echo $router->getControllerName();
	 *</code>
	 *
	 */
	
	class Router extends \Phalcon\Router implements \Phalcon\RouterInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Events\EventsAwareInterface, \Phalcon\Mvc\RouterInterface {

		const MODE_DEFAULT = 0;

		const MODE_NONE = 1;

		const MODE_REST = 2;

		const URI_SOURCE_GET_URL = 0;

		const URI_SOURCE_SERVER_REQUEST_URI = 1;

		protected $_uriSource;

		protected $_routes;

		protected $_routesNameLookup;

		protected $_matchedRoute;

		protected $_matches;

		protected $_wasMatched;

		protected $_removeExtraSlashes;

		protected $_notFoundPaths;

		protected $_isExactControllerName;

		/**
		 * \Phalcon\Mvc\Router constructor
		 *
		 * @param boolean $defaultRoutes
		 */
		public function __construct($defaultRoutes=null){ }


		/**
		 * Get rewrite info. This info is read from $_GET['_url']. This returns '/' if the rewrite information cannot be read
		 *
		 * @return string
		 */
		public function getRewriteUri(){ }


		/**
		 * Sets the URI source. One of the URI_SOURCE_* constants
		 *
		 *<code>
		 *	$router->setUriSource(Router::URI_SOURCE_SERVER_REQUEST_URI);
		 *</code>
		 *
		 * @param int $uriSource
		 * @return \Phalcon\Mvc\Router
		 */
		public function setUriSource($uriSource){ }


		/**
		 * Set whether router must remove the extra slashes in the handled routes
		 *
		 * @param boolean $remove
		 * @return \Phalcon\Mvc\Router
		 */
		public function removeExtraSlashes($remove){ }


		/**
		 * Sets an array of default paths. If a route is missing a path the router will use the defined here
		 * This method must not be used to set a 404 route
		 *
		 *<code>
		 * $router->setDefaults(array(
		 *		'module' => 'common',
		 *		'action' => 'index'
		 * ));
		 *</code>
		 *
		 * @param array $defaults
		 * @return \Phalcon\Mvc\Router
		 */
		public function setDefaults($defaults){ }


		/**
		 * Returns an array of default parameters
		 *
		 * @return array
		 */
		public function getDefaults(){ }


		/**
		 * Handles routing information received from the rewrite engine
		 *
		 *<code>
		 * //Read the info from the rewrite engine
		 * $router->handle();
		 *
		 * //Manually passing an URL
		 * $router->handle('/posts/edit/1');
		 *</code>
		 *
		 * @param string $uri
		 */
		public function handle($uri=null){ }


		/**
		 * Adds a route to the router without any HTTP constraint
		 *
		 *<code>
		 * $router->add('/about', 'About::index');
		 *</code>
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @param array $regex
		 * @param string $httpMethods
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function add($pattern, $paths=null, $regex=null, $httpMethods=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is GET
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addGet($pattern, $paths=null, $regex=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is POST
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addPost($pattern, $paths=null, $regex=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is PUT
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addPut($pattern, $paths=null, $regex=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is PATCH
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addPatch($pattern, $paths=null, $regex=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is DELETE
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addDelete($pattern, $paths=null, $regex=null){ }


		/**
		 * Add a route to the router that only match if the HTTP method is OPTIONS
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addOptions($pattern, $paths=null, $regex=null){ }


		/**
		 * Adds a route to the router that only match if the HTTP method is HEAD
		 *
		 * @param string $pattern
		 * @param string/array $paths
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function addHead($pattern, $paths=null, $regex=null){ }


		/**
		 * Mounts a group of routes in the router
		 *
		 * @param \Phalcon\Mvc\Router\Group $route
		 * @return \Phalcon\Mvc\Router
		 */
		public function mount(\Phalcon\Mvc\Router\Group $group){ }


		/**
		 * Set a group of paths to be returned when none of the defined routes are matched
		 *
		 * @param array|string $paths
		 * @return \Phalcon\Mvc\Router
		 */
		public function notFound($paths){ }


		/**
		 * Removes all the pre-defined routes
		 */
		public function clear(){ }


		/**
		 * Returns the route that matchs the handled URI
		 *
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function getMatchedRoute(){ }


		/**
		 * Returns the sub expressions in the regular expression matched
		 *
		 * @return array
		 */
		public function getMatches(){ }


		/**
		 * Checks if the router macthes any of the defined routes
		 *
		 * @return boolean
		 */
		public function wasMatched(){ }


		/**
		 * Returns all the routes defined in the router
		 *
		 * @return \Phalcon\Mvc\Router\Route[]
		 */
		public function getRoutes(){ }


		/**
		 * Returns a route object by its id
		 *
		 * @param string $id
		 * @return \Phalcon\Mvc\Router\Route | false
		 */
		public function getRouteById($id){ }


		/**
		 * Returns a route object by its name
		 *
		 * @param string $name
		 * @return \Phalcon\Mvc\Router\Route
		 */
		public function getRouteByName($name){ }


		/**
		 * Returns whether controller name should not be mangled
		 */
		public function isExactControllerName(){ }


		/**
		 * Sets the default controller name
		 *
		 * @param string $controllerName
		 */
		public function setDefaultController($handlerName){ }


		/**
		 * Gets the default controller name
		 *
		 * @return string
		 */
		public function getDefaultController(){ }


		/**
		 * Sets the controller name
		 *
		 * @param string $controllerName
		 */
		public function setControllerName($handlerName){ }


		/**
		 * Gets the controller name
		 *
		 * @return string
		 */
		public function getControllerName(){ }

	}
}
