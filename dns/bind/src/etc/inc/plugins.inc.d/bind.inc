<?php

/*
 * Copyright (C) 2018 Michael Muenz <m.muenz@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

function bind_enabled()
{
    $model = new \OPNsense\Bind\General();
    return (string)$model->enabled == '1';
}

function bind_configure()
{
    return [
        'dns' => ['bind_configure_do'],
    ];
}

function bind_services()
{
    $services = [];

    if (!bind_enabled()) {
        return $services;
    }

    $model = new \OPNsense\Bind\General();

    /* DNS service is eligable for core use when both 127.0.0.1 and ::1 are set */
    $localhost4 = false;
    $localhost6 = false;

    foreach (explode(',', (string)$model->listenv4) as $addr) {
        $localhost4 |= $addr === '127.0.0.1';
    }

    foreach (explode(',', (string)$model->listenv6) as $addr) {
        $localhost6 |= $addr === '::1';
    }

    $services[] = [
        /* the port may still be something other than 53, but it's safe to register a conflict for it */
        'dns_ports' => ($localhost4 && $localhost6 ? [(string)$model->port] : []),
        'description' => gettext('BIND Daemon'),
        'configd' => [
            'restart' => ['bind restart'],
            'start' => ['bind start'],
            'stop' => ['bind stop'],
        ],
        'pidfile' => '/var/run/named/pid',
        'name' => 'named',
    ];

    return $services;
}

function bind_xmlrpc_sync()
{
    $result = [];

    $result['id'] = 'bind';
    $result['section'] = 'OPNsense.bind';
    $result['description'] = gettext('BIND domain name service');
    $result['services'] = ['named'];

    return [$result];
}

function bind_configure_do($verbose)
{
    service_log('Starting BIND...', $verbose);

    configd_run('template reload OPNsense/Bind');
    configd_run('bind restart');

    service_log("done.\n", $verbose);
}
