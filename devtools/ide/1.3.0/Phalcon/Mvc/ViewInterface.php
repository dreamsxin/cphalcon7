<?php 

namespace Phalcon\Mvc {

	/**
	 * Phalcon\Mvc\ViewInterface initializer
	 */
	
	interface ViewInterface {

		/**
		 * Sets views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 * @param string $viewsDir
		 */
		public function setViewsDir($viewsDir);


		/**
		 * Gets views directory
		 *
		 * @return string
		 */
		public function getViewsDir();


		/**
		 * Sets the layouts sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 * @param string $layoutsDir
		 */
		public function setLayoutsDir($layoutsDir);


		/**
		 * Gets the current layouts sub-directory
		 *
		 * @return string
		 */
		public function getLayoutsDir();


		/**
		 * Sets a partials sub-directory. Must be a directory under the views directory. Depending of your platform, always add a trailing slash or backslash
		 *
		 * @param string $partialsDir
		 */
		public function setPartialsDir($partialsDir);


		/**
		 * Gets the current partials sub-directory
		 *
		 * @return string
		 */
		public function getPartialsDir();


		/**
		 * Sets base path. Depending of your platform, always add a trailing slash or backslash
		 *
		 * @param string $basePath
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setBasePath($basePath);


		/**
		 * Gets base path
		 *
		 * @return string
		 */
		public function getBasePath();


		/**
		 * Gets the current render level
		 *
		 * @return string
		 */
		public function getCurrentRenderLevel();


		/**
		 * Gets the render level for the view
		 *
		 * @return string
		 */
		public function getRenderLevel();


		/**
		 * Sets the render level for the view
		 *
		 * @param string $level
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setRenderLevel($level);


		/**
		 * Sets the render level for the view
		 *
		 * @param string $level
		 */
		public function disableLevel($level);


		public function getDisabledLevels();


		/**
		 * Disables a specific level of rendering
		 *
		 * @param int|array $level
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setMainView($viewPath);


		/**
		 * Returns the name of the main view
		 *
		 * @return string
		 */
		public function getMainView();


		/**
		 * Change the layout to be used instead of using the name of the latest controller name
		 *
		 * @param string $layout
		 */
		public function setLayout($layout);


		/**
		 * Returns the name of the main view
		 *
		 * @return string
		 */
		public function getLayout();


		/**
		 * Appends template before controller layout
		 *
		 * @param string|array $templateBefore
		 */
		public function setTemplateBefore($templateBefore);


		/**
		 * Resets any template before layouts
		 *
		 */
		public function cleanTemplateBefore();


		/**
		 * Appends template after controller layout
		 *
		 * @param string|array $templateAfter
		 */
		public function setTemplateAfter($templateAfter);


		/**
		 * Resets any template before layouts
		 *
		 */
		public function cleanTemplateAfter();


		/**
		 * Adds parameters to views (alias of setVar)
		 *
		 * @param string $key
		 * @param mixed $value
		 */
		public function setParamToView($key, $value);


		/**
		 * Returns parameters to views
		 *
		 * @return array
		 */
		public function getParamsToView();


		/**
		 * Set all the render params
		 *
		 * @param array $params
		 * @param boolean $merge
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setVars($params, $merge=null);


		/**
		 * Adds parameters to views
		 *
		 * @param string $key
		 * @param mixed $value
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setVar($key, $value);


		/**
		 * Returns a parameter previously set in the view
		 *
		 * @param string $key
		 * @return mixed
		 */
		public function getVar($key);


		/**
		 * Sets the controller name to be view
		 *
		 * @param string $controllerName
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setControllerName($controllerName);


		/**
		 * Gets the name of the controller rendered
		 *
		 * @return string
		 */
		public function getControllerName();


		/**
		 * Sets the action name to be view
		 *
		 * @param string $actionName
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setActionName($actionName);


		/**
		 * Gets the name of the action rendered
		 *
		 * @return string
		 */
		public function getActionName();


		/**
		 * Sets the extra parameters to be view
		 *
		 * @param array $params
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setParams($params);


		/**
		 * Gets extra parameters of the action rendered
		 *
		 * @return array
		 */
		public function getParams();


		/**
		 * Starts rendering process enabling the output buffering
		 */
		public function start();


		/**
		 * Register templating engines
		 *
		 * @param array $engines
		 */
		public function registerEngines($engines);


		public function getRegisteredEngines();


		/**
		 * Returns the registered templating engines, if none is registered it will use \Phalcon\Mvc\View\Engine\Php
		 *
		 * @return array
		 */
		public function getEngines();


		/**
		 * Checks whether a view file exists
		 *
		 * @param string $view
		 * @param boolean $absolutePath
		 * @return boolean
		 */
		public function exists($view, $absolute_path=null);


		/**
		 * Executes render process from dispatching data
		 *
		 * @param string $controllerName
		 * @param string $actionName
		 * @param array $params
		 */
		public function render($controllerName, $actionName, $params=null, $namespace=null, $viewModel=null);


		/**
		 * Choose a view different to render than last-controller/last-action
		 *
		 * @param string $renderView
		 */
		public function pick($renderView);


		/**
		 * Renders a partial view
		 *
		 * @param string $partialPath
		 * @return string
		 */
		public function partial($partialPath);


		/**
		 * Perform the automatic rendering returning the output as a string
		 *
		 * @param string $controllerName
		 * @param string $actionName
		 * @param array $params
		 * @param mixed $configCallback
		 * @return string
		 */
		public function getRender($controllerName, $actionName, $params=null, $configCallback=null);


		/**
		 * Finishes the render process by stopping the output buffering
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function finish();


		/**
		 * Check if the component is currently caching the output content
		 *
		 * @return boolean
		 */
		public function isCaching();


		/**
		 * Returns the cache instance used to cache
		 *
		 * @return \Phalcon\Cache\BackendInterface
		 */
		public function getCache();


		/**
		 * Cache the actual view render to certain level
		 *
		 * @param boolean|array $options
		 */
		public function cache($options=null);


		/**
		 * Externally sets the view content
		 *
		 * @param string $content
		 */
		public function setContent($content);


		/**
		 * Returns cached ouput from another view stage
		 *
		 * @return string
		 */
		public function getContent();


		/**
		 * Start a new section block
		 *
		 * @param string $name
		 */
		public function startSection($name);


		/**
		 * Stop the current section block
		 */
		public function stopSection();


		/**
		 * Returns the content for a section block
		 *
		 * @param  string $name Section name
		 * @param  string $default Default section content
		 * @return string
		 */
		public function section($name, $defaultValue=null);


		/**
		 * Returns the path of the view that is currently rendered
		 *
		 * @return string
		 */
		public function getActiveRenderPath();


		/**
		 * Disables the auto-rendering process
		 *
		 */
		public function disable();


		/**
		 * Enables the auto-rendering process
		 *
		 */
		public function enable();


		/**
		 * Whether automatic rendering is enabled
		 *
		 * @return boolean
		 */
		public function isDisabled();


		/**
		 * Enables namespace view render
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function enableNamespaceView();


		/**
		 * Disables namespace view render
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function disableNamespaceView();


		/**
		 * Enables to lower case view path
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function enableLowerCase();


		/**
		 * Disables to lower case view path
		 *
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function disableLowerCase();


		/**
		 * Adds a converter
		 *
		 * @param string $name
		 * @param callable $converter
		 * @return \Phalcon\Mvc\ViewInterface
		 */
		public function setConverter($name, $converter);


		/**
		 * Returns the router converter
		 *
		 * @return callable|null
		 */
		public function getConverter($name);


		/**
		 * Resets the view component to its factory default values
		 *
		 */
		public function reset();

	}
}
