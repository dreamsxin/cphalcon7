<?php

/*
  +------------------------------------------------------------------------+
  | Phalcon Developer Tools                                                |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2016 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

namespace Phalcon\Commands\Builtin;

use Phalcon\Cli\Color;
use Phalcon\Commands\Command;
use Phalcon\Builder\Project as ProjectBuilder;

/**
 * Project Command
 *
 * Creates project skeletons
 *
 * @package Phalcon\Commands\Builtin
*/
class Project extends Command
{
    /**
     * {@inheritdoc}
     *
     * @return array
     */
    public function getPossibleParams()
    {
        return [
            'name'				=> 'Name of the new project',
            'directory=s'		=> 'Base path on which project will be created [optional]',
            'type=s'			=> 'Type of the application to be generated (cli, micro, simple, modules)',
            'template-path=s'	=> 'Specify a template path [optional]',
			'template-engine=s' => 'Define the template engine, default php (php, volt) [optional]',
            'use-config-ini'	=> 'Use a ini file as configuration file [optional]',
            'adapter=s'			=> 'Define database adapter, default Mysql [optional]',
            'username=s'		=> 'Define database username, default root [optional]',
            'password=s'		=> 'Define database password, default empty [optional]',
            'dbname=s'			=> 'Define database dbname, default same project name [optional]',
            'trace'				=> 'Shows the trace of the framework in case of exception [optional]',
            'help'				=> 'Shows this help'
        ];
    }

    /**
     * {@inheritdoc}
     *
     * @param array $parameters
     * @return mixed
     */
    public function run(array $parameters)
    {
        $projectName = $this->getOption(['name', 1], null, 'default');
        $projectType = $this->getOption(['type', 2], null, 'simple');
        $projectPath = $this->getOption(['directory', 3]);
        $templatePath = $this->getOption(['template-path'], null, TEMPLATE_PATH);
        $templateEngine = $this->getOption(['template-engine'], null, 'php');
        $useConfigIni = $this->getOption('use-config-ini');
        $adapter = $this->getOption('adapter');
        $username = $this->getOption('username');
        $password = $this->getOption('password');
        $dbname = $this->getOption('dbname');

        $builder = new ProjectBuilder([
            'name'				=> $projectName,
            'type'				=> $projectType,
            'directory'			=> $projectPath,
            'templatePath'		=> $templatePath,
			'templateEngine'	=> $templateEngine,
            'useConfigIni'		=> $useConfigIni,
            'adapter'			=> $adapter,
            'username'			=> $username,
            'password'			=> $password,
            'dbname'			=> $dbname
        ]);

        return $builder->build();
    }

    /**
     * {@inheritdoc}
     *
     * @return array
     */
    public function getCommands()
    {
        return ['project', 'create-project'];
    }

    /**
     * {@inheritdoc}
     *
     * @return boolean
     */
    public function canBeExternal()
    {
        return true;
    }

    /**
     * {@inheritdoc}
     *
     * @return void
     */
    public function getHelp()
    {
        print Color::head('Help:') . PHP_EOL;
        print Color::colorize('  Creates a project') . PHP_EOL . PHP_EOL;

        print Color::head('Usage:') . PHP_EOL;
        print Color::colorize('  project [name] [type] [directory]', Color::FG_GREEN) . PHP_EOL . PHP_EOL;

        print Color::head('Arguments:') . PHP_EOL;
        print Color::colorize('  help', Color::FG_GREEN);
        print Color::colorize("\tShows this help text") . PHP_EOL . PHP_EOL;

        print Color::head('Example') . PHP_EOL;
        print Color::colorize('  phalcon project store simple', Color::FG_GREEN) . PHP_EOL . PHP_EOL;

        $this->printParameters($this->getPossibleParams());
    }

    /**
     * {@inheritdoc}
     *
     * @return integer
     */
    public function getRequiredParams()
    {
        return 1;
    }
}
