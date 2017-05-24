<?php 

namespace Phalcon\Process {

	/**
	 * Phalcon\Process\Proc
	 *
	 * Pro for Phalcon\Process procs
	 */
	
	class Proc {

		const MODE_NONE = 0;

		const MODE_TTY = 1;

		const MODE_PTY = 2;

		const STATUS_READY = 0;

		const STATUS_STARTED = 1;

		const STATUS_STOP = 2;

		const STATUS_TERMINATED = 3;

		const STDIN = 0;

		const STDOUT = 1;

		const STDERR = 2;

		const SIGKILL = 9;

		const SIGTERM = 15;

		protected $_command;

		protected $_cwd;

		protected $_env;

		protected $_timeout;

		protected $_options;

		protected $_mode;

		protected $_process;

		protected $_pid;

		protected $_status;

		protected $_information;

		protected $_signaled;

		protected $termsig;

		protected $_latestSignal;

		protected $_startTime;

		protected $_exitCode;

		protected $_running;

		protected $_blocking;

		protected $_pipes;

		/**
		 * \Phalcon\Process\Proc constructor
		 *
		 * @param string $command     The command line to run
		 * @param int $pid            The process identifier
		 * @param string $cwd         The working directory or null to use the working dir of the current PHP process
		 * @param array $env          The environment variables or null to use the same environment as the current PHP process
		 * @param float $timeout      The timeout in seconds or null to disable
		 * @param array $options
		 */
		public function __construct($command, $cwd=null, $env=null, $timeout=null, $options=null){ }


		public function __destruct(){ }


		/**
		 * Sets the mode.
		 */
		public function setMode($mode){ }


		/**
		 * Creates the descriptors needed by the proc_open.
		 *
		 * @return array
		 */
		public function getDescriptors(){ }


		/**
		 * Returns whether PTY is supported on the current operating system.
		 *
		 * @return boolean
		 */
		public function isPtySupported(){ }


		/**
		  * Starts the process and returns after writing the input to STDIN.
		  *
		  * This method blocks until all STDIN data is sent to the process then it
		  * returns while the process runs in the background.
		  *
		  * The termination of the process can be awaited with wait().
		  *
		  * The callback receives the type of output (out or err) and some bytes from
		  * the output in real-time while writing the standard input to the process.
		  * It allows to have feedback from the independent process during execution.
		  *
		  *
		  * @throws \Phalcon\Process\Exception When process can't be launched
		  * @throws \Phalcon\Process\Exception When process is already running
		  */
		public function start(){ }


		/**
		 * Stops the process.
		 *
		 * @param int|float $timeout The timeout in seconds
		 * @param int       $signal  A POSIX signal to send in case the process has not stop at timeout, default is SIGKILL (9)
		 *
		 * @return boolean
		 */
		public function stop($timeout=null, $signal=null){ }


		public function close(){ }


		/**
		 * Restarts the process
		 */
		public function reStart(){ }


		/**
		 * Checks if the process is currently running.
		 *
		 * @return boolean true if the process is currently running, false otherwise
		 */
		public function isRunning(){ }


		/**
		 * Updates the status of the process, reads pipes.
		 *
		 * @param boolean $blocking Whether to use a blocking read call
		 * @return boolean
		 */
		public function update($blocking=null){ }


		public function getCommandForPid($pid){ }


		public function getStarttimeForPid($pid){ }


		public function getStatForPid($pid){ }


		/**
		 * Performs a check between the timeout definition and the time the process started
		 *
		 * In case you run a background process (with the start method), you should
		 * trigger this method regularly to ensure the process timeout
		 *
		 * @throws \Phalcon\Process\Exception In case the timeout was reached
		 */
		public function checkTimeout(){ }


		/**
		 * Returns the Pid (process identifier), if applicable.
		 *
		 * @return int|null The process id if running, null otherwise
		 */
		public function getPid(){ }


		/**
		 * Waits for the process to terminate
		 *
		 * @return boolean
		 */
		public function wait(){ }


		/**
		 * Reads data in file handles and pipes
		 *
		 * @param int $type
		 * @param boolean $blocking
		 * @return boolean
		 */
		public function read($type, $blocking){ }


		/**
		 * Writes data to stdin
		 *
		 * @return boolean
		 */
		public function write($data, $type=null){ }


		/**
		 * Handle the process
		 *
		 * @param callable $onrecv
		 * @param callable $onsend
		 * @param callable $onerror
		 * @param callable $ontimeout
		 * @param int $timeout
		 */
		public function handle($onrecv=null, $onsend=null, $onerror=null, $ontimeout=null, $timeout=null){ }


		/**
		 * Returns true if a system call has been interrupted
		 *
		 * @return bool
		 */
		protected function hasSystemCallBeenInterrupted(){ }


		/**
		 * Set streams to blocking / non blocking
		 *
		 * @return boolean
		 */
		public function setBlocking($flag){ }

	}
}
