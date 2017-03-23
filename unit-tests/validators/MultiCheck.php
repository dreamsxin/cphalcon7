<?php
use Phalcon\Validation\Validator,
	Phalcon\Validation\ValidatorInterface;

class MultiCheck extends Validator implements ValidatorInterface
{
	public function validate(Phalcon\ValidationInterface $validation, $attributes, bool $allowEmpty = NULL)
	{
		if (!is_array($attributes)) {
			throw new Exception("Field must be array");
		}
		$type = 'MultiCheck';
		$values = array();
		foreach($attributes as $attribute) {
			$values[] = $validation->getValue($attribute);
		}

		if (!$this->valid($values)) {
			$label = $this->getOption('label');
			$num = $this->getOption('num');
			if (empty($label)) {
				$label = $validation->getLabel($attributes);
			}
			$pairs = array(':field' => $label, ':num' => $num);
			$message = $this->getOption('message');
			if (empty($message)) {
				$message = $validation->getDefaultMessage($type, "Field :field must set more than :num");
			}
			$prepared = strtr($message, $pairs);
			$validation->appendMessage(new Phalcon\Validation\Message($prepared, $attribute, $type));
			return FALSE;
		}
		return TRUE;
	}

	public function valid($values) {
		$num = $this->getOption('num');
		return count(array_filter($values)) >= $num;
	}
}
