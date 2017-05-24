<?php 

namespace Phalcon\Date {

	/**
	 * Phalcon\Date\DateTime
	 *
	 *<code>
	 * $datetime = new \Phalcon\Date\DateTime();
	 * $start = $datetime->startOfDay();
	 * $end = $datetime->endOfDay();
	 *</code>
	 */
	
	class DateTime extends \DateTime implements \DateTimeInterface {

		const ATOM = Y-m-d\TH:i:sP;

		const COOKIE = l, d-M-Y H:i:s T;

		const ISO8601 = Y-m-d\TH:i:sO;

		const RFC822 = D, d M y H:i:s O;

		const RFC850 = l, d-M-y H:i:s T;

		const RFC1036 = D, d M y H:i:s O;

		const RFC1123 = D, d M Y H:i:s O;

		const RFC2822 = D, d M Y H:i:s O;

		const RFC3339 = Y-m-d\TH:i:sP;

		const RFC3339_EXTENDED = Y-m-d\TH:i:s.vP;

		const RSS = D, d M Y H:i:s O;

		const W3C = Y-m-d\TH:i:sP;

		/**
		 * Create a new instance
		 *
		 * @param string|null $time
		 * @param \DateTimeZone|string|null $tz
		 */
		public function __construct($font){ }


		/**
		 * Set the date and time all together
		 *
		 * @param int $year
		 * @param int $month
		 * @param int $day
		 * @param int $hour
		 * @param int $minute
		 * @param int $second
		 *
		 * @return \DateTime
		 */
		public function setDateTime($year, $month, $day, $hour, $minute, $second=null){ }


		/**
		 * Resets the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfDay(){ }


		/**
		 * Resets the time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfDay(){ }


		/**
		 * Resets the date to the first day of the month and the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfMonth(){ }


		/**
		 * Resets the date to end of the month and time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfMonth(){ }


		/**
		 * Resets the date to the first day of the quarter and the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfQuarter(){ }


		/**
		 * Resets the date to end of the quarter and time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfQuarter(){ }


		/**
		 * Resets the date to the first day of the year and the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfYear(){ }


		/**
		 * Resets the date to end of the year and time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfYear(){ }


		/**
		 * Resets the date to the first day of the year and the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfDecade(){ }


		/**
		 * Resets the date to end of the decade and time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfDecade(){ }


		/**
		 * Resets the date to the first day of the century and the time to 00:00:00
		 *
		 * @return \DateTime
		 */
		public function startOfCentury(){ }


		/**
		 * Resets the date to end of the century and time to 23:59:59
		 *
		 * @return \DateTime
		 */
		public function endOfCentury(){ }


		/**
		 * Add or Remove years from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyYear($year){ }


		/**
		 * Add or Remove quarters from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyQuarter($quarter){ }


		/**
		 * Add or Remove months from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyMonth($month){ }


		/**
		 * Add or Remove days from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyDay($day){ }


		/**
		 * Add or Remove hours from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyHour($hour){ }


		/**
		 * Add or Remove minutes from the instance
		 *
		 * @return \DateTime
		 */
		public function modifyMinute($minute){ }


		/**
		 * Add or Remove seconds from the instance
		 *
		 * @return \DateTime
		 */
		public function modifySecond($second){ }


		/**
		 * Get a part of the Carbon object
		 *
		 * @param string $name
		 *
		 * @throws \InvalidArgumentException
		 *
		 * @return string|int|\DateTimeZone
		 */
		public function __get($property){ }

	}
}
