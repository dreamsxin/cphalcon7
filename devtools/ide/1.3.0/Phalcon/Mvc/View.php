<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\View
	 *
	 * Phalcon\Mvc\View is a class for working with the "view" portion of the model-view-controller pattern.
	 * That is, it exists to help keep the view script separate from the model and controller scripts.
	 * It provides a system of helpers, output filters, and variable escaping.
	 *
	 * <code>
	 * //Setting views directory
	 * $view = new Phalcon\Mvc\View();
	 * $view->setViewsDir('app/views/');
	 *
	 * $view->start();
	 * //Shows recent posts view (app/views/posts/recent.phtml)
	 * $view->render('posts', 'recent');
	 * $view->finish();
	 *
	 * //Printing views output
	 * echo $view->getContent();
	 * </code>
	 */
	
	class View extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Phalcon\Mvc\ViewInterface {

		const LEVEL_MAIN_LAYOUT = 6;

		const LEVEL_AFTER_TEMPLATE = 5;

		const LEVEL_NAMESPACE = 4;

		const LEVEL_CONTROLLER = 3;

		const LEVEL_LAYOUT = 3;

		const LEVEL_BEFORE_TEMPLATE = 2;

		const LEVEL_ACTION_VIEW = 1;

		const LEVEL_NO_RENDER = 0;

		const CACHE_MODE_NONE = ;

		const CACHE_MODE_INVERSE = 1;

		protected $_options;

		protected $_basePath;

		protected $_content;

		protected $_sections;

		protected $_renderLevel;

		protected $_currentRenderLevel;

		protected $_disabledLevels;

		protected $_viewParams;

		protected $_layout;

		protected $_layoutsDir;

		protected $_partialsDir;

		protected $_enableLayoutsAbsolutePath;

		protected $_enablePartialsAbsolutePath;

		protected $_viewsDir;

		protected $_enableNamespaceView;

		protected $_templatesBefore;

		protected $_templatesAfter;

		protected $_engines;

		protected $_registeredEngines;

		protected $_mainView;

		protected $_controllerName;

		protected $_namespaceName;

		protected $_actionName;

		protected $_params;

		protected $_pickView;

		protected $_cache;

		protected $_cacheLevel;

		protected $_cacheMode;

		protected $_activeRenderPath;

		protected $_disabled;

		protected $_lowerCase;

		protected $_converters;

		/**
		 * \Phalcon\Mvc\View constructor
		 *
		 * @param array $options
		 */
		public function __construct($options=null){ }


		/**
		 * Sets views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 * @param string $viewsDir
		 * @return \Phalcon\Mvc\View
		 */
		public function setViewsDir($viewsDir){ }


		/**
		 * Gets views directory
		 *
		 * @return string
		 */
		public function getViewsDir(){ }


		/**
		 * Sets the layouts sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 *<code>
		 * $view->setLayoutsDir('../common/layouts/');
		 *</code>
		 *
		 * @param string $layoutsDir
		 * @return \Phalcon\Mvc\View
		 */
		public function setLayoutsDir($layoutsDir){ }


		/**
		 * Gets the current layouts sub-directory
		 *
		 * @return string
		 */
		public function getLayoutsDir(){ }


		/**
		 * Sets a partials sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 *<code>
		 * $view->setPartialsDir('../common/partials/');
		 *</code>
		 *
		 * @param string $partialsDir
		 * @return \Phalcon\Mvc\View
		 */
		public function setPartialsDir($partialsDir){ }


		/**
		 * Gets the current partials sub-directory
		 *
		 * @return string
		 */
		public function getPartialsDir(){ }


		/**
		 * Sets base path. Depending of your platform, always add a trailing slash or backslash
		 *
		 * <code>
		 * 	$view->setBasePath(__DIR__ . '/');
		 * </code>
		 *
		 * @param string|array $basePath
		 * @return \Phalcon\Mvc\View
		 */
		public function setBasePath($basePath){ }


		/**
		 * Gets base path
		 *
		 * @return string
		 */
		public function getBasePath(){ }


		/**
		 * Returns the render level for the view
		 *
		 * @return int
		 */
		public function getCurrentRenderLevel(){ }


		/**
		 * Returns the render level for the view
		 *
		 * @return int
		 */
		public function getRenderLevel(){ }


		/**
		 * Sets the render level for the view
		 *
		 * <code>
		 * 	//Render the view related to the controller only
		 * 	$this->view->setRenderLevel(View::LEVEL_LAYOUT);
		 * </code>
		 *
		 * @param string $level
		 * @return \Phalcon\Mvc\View
		 */
		public function setRenderLevel($level){ }


		/**
		 * Disables a specific level of rendering
		 *
		 *<code>
		 * //Render all levels except ACTION level
		 * $this->view->disableLevel(View::LEVEL_ACTION_VIEW);
		 *</code>
		 *
		 * @param int|array $level
		 * @return \Phalcon\Mvc\View
		 */
		public function disableLevel($level){ }


		/**
		 * Returns an array with disabled render levels
		 *
		 * @return array
		 */
		public function getDisabledLevels(){ }


		/**
		 * Sets default view name. Must be a file without extension in the views directory
		 *
		 * <code>
		 * 	//Renders as main view views-dir/base.phtml
		 * 	$this->view->setMainView('base');
		 * </code>
		 *
		 * @param string $viewPath
		 * @return \Phalcon\Mvc\View
		 */
		public function setMainView($viewPath){ }


		/**
		 * Returns the name of the main view
		 *
		 * @return string
		 */
		public function getMainView(){ }


		/**
		 * Change the layout to be used instead of using the name of the latest controller name
		 *
		 * <code>
		 * 	$this->view->setLayout('main');
		 * </code>
		 *
		 * @param string $layout
		 * @return \Phalcon\Mvc\View
		 */
		public function setLayout($layout){ }


		/**
		 * Returns the name of the main view
		 *
		 * @return string
		 */
		public function getLayout(){ }


		/**
		 * Appends template before controller layout
		 *
		 * @param string|array $templateBefore
		 * @return \Phalcon\Mvc\View
		 */
		public function setTemplateBefore($templateBefore){ }


		/**
		 * Resets any template before layouts
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function cleanTemplateBefore(){ }


		/**
		 * Appends template after controller layout
		 *
		 * @param string|array $templateAfter
		 * @return \Phalcon\Mvc\View
		 */
		public function setTemplateAfter($templateAfter){ }


		/**
		 * Resets any template after layouts
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function cleanTemplateAfter(){ }


		/**
		 * Adds parameters to views (alias of setVar)
		 *
		 *<code>
		 *	$this->view->setParamToView('products', $products);
		 *</code>
		 *
		 * @param string $key
		 * @param mixed $value
		 * @return \Phalcon\Mvc\View
		 */
		public function setParamToView($key, $value){ }


		/**
		 * Returns parameters to views
		 *
		 * @return array
		 */
		public function getParamsToView(){ }


		/**
		 * Set all the render params
		 *
		 *<code>
		 *	$this->view->setVars(array('products' => $products));
		 *</code>
		 *
		 * @param array $params
		 * @param boolean $merge
		 * @return \Phalcon\Mvc\View
		 */
		public function setVars($params, $merge=null){ }


		/**
		 * Set a single view parameter
		 *
		 *<code>
		 *	$this->view->setVar('products', $products);
		 *</code>
		 *
		 * @param string $key
		 * @param mixed $value
		 * @return \Phalcon\Mvc\View
		 */
		public function setVar($key, $value){ }


		/**
		 * Returns a parameter previously set in the view
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function getVar($key){ }


		/**
		 * Sets the controller name to be view
		 *
		 * @param string $controllerName
		 * @return \Phalcon\Mvc\View
		 */
		public function setControllerName($controllerName){ }


		/**
		 * Gets the name of the controller rendered
		 *
		 * @return string
		 */
		public function getControllerName(){ }


		/**
		 * Sets the action name to be view
		 *
		 * @param string $actionName
		 * @return \Phalcon\Mvc\View
		 */
		public function setActionName($actionName){ }


		/**
		 * Gets the name of the action rendered
		 *
		 * @return string
		 */
		public function getActionName(){ }


		/**
		 * Sets the extra parameters to be view
		 *
		 * @param array $params
		 * @return \Phalcon\Mvc\View
		 */
		public function setParams($params){ }


		/**
		 * Gets extra parameters of the action rendered
		 *
		 * @return array
		 */
		public function getParams(){ }


		public function setNamespaceName(){ }


		public function getNamespaceName(){ }


		/**
		 * Starts rendering process enabling the output buffering
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function start(){ }


		/**
		 * Loads registered template engines, if none is registered it will use \Phalcon\Mvc\View\Engine\Php
		 *
		 * @return array
		 */
		protected function _loadTemplateEngines(){ }


		/**
		 * Checks whether view exists on registered extensions and render it
		 *
		 * @param array $engines
		 * @param string $viewPath
		 * @param boolean $silence
		 * @param boolean $mustClean
		 */
		protected function _engineRender(){ }


		/**
		 * Register templating engines
		 *
		 *<code>
		 *$this->view->registerEngines(array(
		 *  ".phtml" => "Phalcon\Mvc\View\Engine\Php",
		 *  ".volt" => "Phalcon\Mvc\View\Engine\Volt",
		 *  ".mhtml" => "MyCustomEngine"
		 *));
		 *</code>
		 *
		 * @param array $engines
		 * @return \Phalcon\Mvc\View
		 */
		public function registerEngines($engines){ }


		/**
		 * Returns the registered templating engines
		 *
		 * @brief array \Phalcon\Mvc\View::getRegisteredEngines()
		 */
		public function getRegisteredEngines(){ }


		/**
		 * Returns the registered templating engines, if none is registered it will use \Phalcon\Mvc\View\Engine\Php
		 *
		 * @return array
		 */
		public function getEngines(){ }


		/**
		 * Checks whether a view file exists
		 *
		 * @param string $view
		 * @param boolean $absolutePath
		 * @return boolean
		 */
		public function exists($view, $absolute_path=null){ }


		/**
		 * Executes render process from dispatching data
		 *
		 *<code>
		 * //Shows recent posts view (app/views/posts/recent.phtml)
		 * $view->start()->render('posts', 'recent')->finish();
		 *</code>
		 *
		 * @param string $controllerName
		 * @param string $actionName
		 * @param array $params
		 * @param string $namespace_name
		 * @param \Phalcon\Mvc\View\ModelInterface $viewModel
		 * @return \Phalcon\Mvc\View
		 */
		public function render($controllerName, $actionName, $params=null, $namespace=null, $viewModel=null){ }


		/**
		 * Choose a different view to render instead of last-controller/last-action
		 *
		 * <code>
		 * class ProductsController extends \Phalcon\Mvc\Controller
		 * {
		 *
		 *    public function saveAction()
		 *    {
		 *
		 *         //Do some save stuff...
		 *
		 *         //Then show the list view
		 *         $this->view->pick("products/list");
		 *    }
		 * }
		 * </code>
		 *
		 * @param string|array $renderView
		 * @return \Phalcon\Mvc\View
		 */
		public function pick($renderView){ }


		/**
		 * Renders a partial view
		 *
		 * <code>
		 * 	//Show a partial inside another view
		 * 	$this->partial('shared/footer');
		 * </code>
		 *
		 * <code>
		 * 	//Show a partial inside another view with parameters
		 * 	$this->partial('shared/footer', array('content' => $html));
		 * </code>
		 *
		 * @param string $partialPath
		 * @param array $params
		 * @param boolean $autorender
		 */
		public function partial($partialPath){ }


		/**
		 * Perform the automatic rendering returning the output as a string
		 *
		 * <code>
		 * 	$template = $this->view->getRender('products', 'show', array('products' => $products));
		 * </code>
		 *
		 * @param string $controllerName
		 * @param string $actionName
		 * @param array $params
		 * @param mixed $configCallback
		 * @return string
		 */
		public function getRender($controllerName, $actionName, $params=null, $configCallback=null){ }


		/**
		 * Finishes the render process by stopping the output buffering
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function finish(){ }


		/**
		 * Create a \Phalcon\Cache based on the internal cache options
		 *
		 * @return \Phalcon\Cache\BackendInterface
		 */
		protected function _createCache(){ }


		/**
		 * Check if the component is currently caching the output content
		 *
		 * @return boolean
		 */
		public function isCaching(){ }


		/**
		 * Returns the cache instance used to cache
		 *
		 * @return \Phalcon\Cache\BackendInterface
		 */
		public function getCache(){ }


		/**
		 * Cache the actual view render to certain level
		 *
		 *<code>
		 *  $this->view->cache(array('key' => 'my-key', 'lifetime' => 86400));
		 *</code>
		 *
		 * @param boolean|array $options
		 * @return \Phalcon\Mvc\View
		 */
		public function cache($options=null){ }


		/**
		 * Externally sets the view content
		 *
		 *<code>
		 *	$this->view->setContent("<h1>hello</h1>");
		 *</code>
		 *
		 * @param string $content
		 * @return \Phalcon\Mvc\View
		 */
		public function setContent($content){ }


		/**
		 * Returns cached output from another view stage
		 *
		 * @return string
		 */
		public function getContent(){ }


		/**
		 * Start a new section block
		 *
		 * @param string $name
		 */
		public function startSection($name){ }


		/**
		 * Stop the current section block
		 *
		 * @return string
		 */
		public function stopSection(){ }


		/**
		 * Returns the content for a section block
		 *
		 * @param string $name
		 * @param string $default
		 * @return string|null
		 */
		public function section($name, $defaultValue=null){ }


		/**
		 * Returns the path of the view that is currently rendered
		 *
		 * @return string
		 */
		public function getActiveRenderPath(){ }


		/**
		 * Disables the auto-rendering process
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function disable(){ }


		/**
		 * Enables the auto-rendering process
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function enable(){ }


		/**
		 * Whether automatic rendering is enabled
		 *
		 * @return boolean
		 */
		public function isDisabled(){ }


		/**
		 * Enables namespace view render
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function enableNamespaceView(){ }


		/**
		 * Disables namespace view render
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function disableNamespaceView(){ }


		/**
		 * Enables to lower case view path
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function enableLowerCase(){ }


		/**
		 * Whether to lower case view path
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function disableLowerCase(){ }


		/**
		 * Adds a converter
		 *
		 * @param string $name
		 * @param callable $converter
		 * @return \Phalcon\Mvc\View
		 */
		public function setConverter($name, $converter){ }


		/**
		 * Returns the router converter
		 *
		 * @return callable|null
		 */
		public function getConverter($name){ }


		/**
		 * Resets the view component to its factory default values
		 *
		 * @return \Phalcon\Mvc\View
		 */
		public function reset(){ }


		/**
		 * Magic method to pass variables to the views
		 *
		 *<code>
		 *	$this->view->products = $products;
		 *</code>
		 *
		 * @param string $key
		 * @param mixed $value
		 */
		public function __set($property, $value){ }


		/**
		 * Magic method to retrieve a variable passed to the view
		 *
		 *<code>
		 *	echo $this->view->products;
		 *</code>
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function __get($property){ }


		/**
		 * Magic method to inaccessible a variable passed to the view
		 *
		 *<code>
		 *	isset($this->view->products)
		 *</code>
		 *
		 * @param string $key
		 * @return boolean
		 */
		public function __isset($property){ }


		public function insert($partialPath){ }

	}
}
