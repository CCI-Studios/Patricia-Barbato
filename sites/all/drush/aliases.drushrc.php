<?php

$aliases['dev'] = array(
	'uri'=> 'dev.patriciabarbato.com',
	'root' => '/home/pbarbato/subdomains/dev/public_html',
	'remote-host'=> 'host.cciserver2.com',
	'remote-user'=> 'pbarbato',
	'path-aliases'=> array(
		'%files'=> 'sites/default/files',
	)
);

$aliases['live'] = array(
	'uri'=> 'live.patriciabarbato.com',
	'root' => '/home/pbarbato/subdomains/live/public_html',
	'remote-host'=> 'host.cciserver2.com',
	'remote-user'=> 'pbarbato',
	'path-aliases'=> array(
		'%files'=> 'sites/default/files',
	)
);
