<?php

class UserEventListener {
	/**
	 * Change order price
	 *
	 * @param Model $user
	 * @param Model $order
	 * @return boolean
	 */
	public function OrderPriceUpdate($event, $user, $order) {
	}

	/**
	 * Change order status
	 *
	 * @param Model $user
	 * @param Model $order
	 * @return boolean
	 */
	public function orderStatusUpdate($event, $user, $order) {
	}
}
