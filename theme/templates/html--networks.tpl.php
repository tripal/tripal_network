<?php
$js_path = url(drupal_get_path('module', 'tripal_network') . '/theme/js', array('absolute' => TRUE));
$css_path = url(drupal_get_path('module', 'tripal_network') . '/theme/css', array('absolute' => TRUE));
$linkurius_path = url(libraries_get_path('linkurious.js'), array('absolute' => TRUE));
?>
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="utf-8">
    <title>Network Viewer</title>

    <!--  Stylesheets -->
    <link rel="stylesheet" href="<?php print $css_path . '/tripal_network_viewer.css'?>" media="screen">
    <link rel="stylesheet" href="//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css" media="screen">

    <!--  Javascript -->
    <?php print $scripts ?>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/sigma.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.layouts.forceLink.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.activeState.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.dragNodes.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.keyboard.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.select.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.locate.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.animate.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.plugins.lasso.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.helpers.graph.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.renderers.edgeLabels.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $linkurius_path . '/dist/plugins/sigma.renderers.linkurious.min.js'?>"></script>
    <script type="text/javascript" src="//code.jquery.com/ui/1.11.4/jquery-ui.js"></script>
    <script type="text/javascript" src="<?php print $js_path . '/chroma.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $js_path . '/tripal_network_viewer.js'?>"></script>

    <?php print $head ?>
  </head>
  <body class="claro">
      <?php print $page ?>
  </body>
</html>