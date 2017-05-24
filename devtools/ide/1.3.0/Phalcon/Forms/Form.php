<?php 

namespace Phalcon\Forms {

	/**
	 * Phalcon\Forms\Form
	 *
	 * This component allows to build forms using an object-oriented interface
	 */
	
	class Form extends \Phalcon\Di\Injectable implements \Phalcon\Events\EventsAwareInterface, \Phalcon\Di\InjectionAwareInterface, \Countable, \Iterator, \Traversable {

		const VALUES_RAW = 1;

		const VALUES_AS_ARRAY = 2;

		protected $_position;

		protected $_entity;

		protected $_options;

		protected $_data;

		protected $_filterData;

		protected $_elements;

		protected $_messages;

		/**
		 * \Phalcon\Forms\Form constructor
		 *
		 * @param object $entity
		 * @param array $userOptions
		 */
		public function __construct($entity=null, $userOptions=null){ }


		/**
		 * Sets the form's action
		 *
		 * @param string $action
		 * @return \Phalcon\Forms\Form
		 */
		public function setAction($action){ }


		/**
		 * Returns the form's action
		 *
		 * @return string
		 */
		public function getAction(){ }


		/**
		 * Sets the form's method
		 *
		 * @param string $action
		 * @return \Phalcon\Forms\Form
		 */
		public function setMethod($method){ }


		/**
		 * Returns the form's method
		 *
		 * @return string
		 */
		public function getMethod(){ }


		/**
		 * Sets the form's enctype
		 *
		 * @param string $action
		 * @return \Phalcon\Forms\Form
		 */
		public function setEnctype($enctype){ }


		/**
		 * Returns the form's enctype
		 *
		 * @return string
		 */
		public function getEnctype(){ }


		/**
		 * Sets an option for the form
		 *
		 * @param string $option
		 * @param mixed $value
		 * @return \Phalcon\Forms\Form
		 */
		public function setOption($option, $value){ }


		/**
		 * Returns the value of an option if present
		 *
		 * @param string $option
		 * @param mixed $defaultValue
		 * @return mixed
		 */
		public function getOption($option, $defaultValue=null){ }


		/**
		 * Sets options for the element
		 *
		 * @param array $options
		 * @return \Phalcon\Forms\ElementInterface
		 */
		public function setOptions($options){ }


		/**
		 * Returns the options for the element
		 *
		 * @return array
		 */
		public function getOptions(){ }


		/**
		 * Sets the entity related to the model
		 *
		 * @param object $entity
		 * @return \Phalcon\Forms\Form
		 */
		public function setEntity($entity){ }


		/**
		 * Returns the entity related to the model
		 *
		 * @return object
		 */
		public function getEntity(){ }


		/**
		 * Returns the form elements added to the form
		 *
		 * @return \Phalcon\Forms\ElementInterface[]
		 */
		public function getElements(){ }


		/**
		 * Binds data to the entity
		 *
		 * @param array $data
		 * @param object $entity
		 * @param array $whitelist
		 * @return \Phalcon\Forms\Form
		 */
		public function bind($data, $entity=null, $whitelist=null){ }


		/**
		 * Validates the form
		 *
		 * @param array $data
		 * @param object $entity
		 * @return boolean
		 */
		public function isValid($data=null, $entity=null){ }


		/**
		 * Returns the messages generated in the validation
		 *
		 * @param boolean $byItemName
		 * @return \Phalcon\Validation\Message\Group
		 */
		public function getMessages($byItemName=null){ }


		/**
		 * Returns the messages generated for a specific element
		 *
		 * @return \Phalcon\Validation\Message\Group[]
		 */
		public function getMessagesFor($name){ }


		/**
		 * Check if messages were generated for a specific element
		 *
		 * @return boolean
		 */
		public function hasMessagesFor($name){ }


		/**
		 * Adds an element to the form
		 *
		 * @param \Phalcon\Forms\ElementInterface $element
		 * @param string $postion
		 * @param bool $type If $type is TRUE, the element wile add before $postion, else is after
		 * @return \Phalcon\Forms\Form
		 */
		public function add($element, $postion=null, $type=null){ }


		/**
		 * Renders a specific item in the form
		 *
		 * @param string $name
		 * @param array $attributes
		 * @param string $template
		 * @return string
		 */
		public function render($name=null, $attributes=null){ }


		/**
		 * Returns an element added to the form by its name
		 *
		 * @param string $name
		 * @return \Phalcon\Forms\ElementInterface
		 */
		public function get($name){ }


		/**
		 * Generate the label of a element added to the form including HTML
		 *
		 * @param string $name
		 * @return string
		 */
		public function label($name, $attributes=null){ }


		/**
		 * Returns a label for an element
		 *
		 * @param string $name
		 * @return string
		 */
		public function getLabel($name){ }


		/**
		 * Gets a value from the internal related entity or from the default value
		 *
		 * @param string $name
		 * @return mixed
		 */
		public function getValue($name, $flag=null){ }


		/**
		 * Gets a values
		 *
		 * @param string $name
		 * @return mixed
		 */
		public function getValues($name=null, $flag=null){ }


		/**
		 * Check if the form contains an element
		 *
		 * @param string $name
		 * @return boolean
		 */
		public function has($name){ }


		/**
		 * Removes an element from the form
		 *
		 * @param string $name
		 * @return boolean
		 */
		public function remove($name){ }


		/**
		 * Clears every element in the form to its default value
		 *
		 * @param array $fields
		 * @return \Phalcon\Forms\Form
		 */
		public function clear($fields=null){ }


		/**
		 * Returns the number of elements in the form
		 *
		 * @return int
		 */
		public function count(){ }


		/**
		 * Rewinds the internal iterator
		 */
		public function rewind(){ }


		/**
		 * Returns the current element in the iterator
		 *
		 * @return \Phalcon\Forms\ElementInterface
		 */
		public function current(){ }


		/**
		 * Returns the current position/key in the iterator
		 *
		 * @return int
		 */
		public function key(){ }


		/**
		 * Moves the internal iteration pointer to the next position
		 *
		 */
		public function next(){ }


		/**
		 * Check if the current element in the iterator is valid
		 *
		 * @return boolean
		 */
		public function valid(){ }


		/**
		 * Appends a message to the form
		 *
		 *<code>
		 * $form->appendMessage('email', new \Phalcon\Validation\Message('Must be not empty '));
		 *</code>
		 *
		 * @param string $field
		 * @param \Phalcon\Validation\Message $message
		 * @return \Phalcon\Forms\Form
		 */
		public function appendMessage($field, $message){ }


		/**
		 * Appends a messages to the form
		 *
		 *<code>
		 * $form->appendMessages('email', array(new \Phalcon\Validation\Message('Must be not empty '), new \Phalcon\Validation\Message('Must be an email address')));
		 *</code>
		 *
		 * @param string $field
		 * @param \Phalcon\Validation\MessageInterface[] $messages
		 * @return \Phalcon\Forms\Form
		 */
		public function appendMessages($field, $messages){ }


		/**
		 * Returns a form elements as an array
		 *
		 * @return array
		 */
		public function toArray(){ }


		/**
		 * Renders the form html
		 *
		 * @return string
		 */
		public function __toString(){ }

	}
}
