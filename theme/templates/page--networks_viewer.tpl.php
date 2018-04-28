<?php
global $user;
$mod_path = drupal_get_path('module', 'tripal_network');

if ($messages) { ?>
  <div id="console" class="clearfix"><?php print $messages; ?></div><?php
}

print drupal_render($page['content']);

