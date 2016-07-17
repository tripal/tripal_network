<?php
$module_path = drupal_get_path('module', 'tripal_network'); ?>
<!DOCTYPE HTML>
<html>
  <head>
    <meta charset="utf-8">
    <title>Network Viewer</title>

    <!--  Stylesheets -->
    <link rel="stylesheet" href="<?php print $module_path . '/theme/css/tripal_network_viewer.css'?>" media="screen">
    <!-- <link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Roboto" media="screen"> -->
    <!-- <link rel="stylesheet" href="http://yui.yahooapis.com/pure/0.6.0/pure-min.css" media="screen"> -->
    <!-- <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css" media="screen"> -->
    <!-- <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap-theme.min.css" media="screen"> -->
    <!-- <link rel="stylesheet" href="http://fonts.googleapis.com/css?family=Lato&subset=latin,latin-ext" media="screen">-->
    <link rel="stylesheet" href="//code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css" media="screen">

    <!--  Javascript -->
    <?php print $scripts ?>
    <!-- <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/2.2.3/jquery.min.js"></script> -->
    <script type="text/javascript" src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/sigma.core.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/conrad.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/utils/sigma.utils.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/utils/sigma.polyfills.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/sigma.settings.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/classes/sigma.classes.dispatcher.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/classes/sigma.classes.configurable.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/classes/sigma.classes.graph.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/classes/sigma.classes.camera.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/classes/sigma.classes.quad.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/captors/sigma.captors.mouse.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/captors/sigma.captors.touch.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/sigma.renderers.canvas.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/sigma.renderers.webgl.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/sigma.renderers.svg.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/sigma.renderers.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/webgl/sigma.webgl.nodes.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/webgl/sigma.webgl.nodes.fast.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/webgl/sigma.webgl.edges.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/webgl/sigma.webgl.edges.fast.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/webgl/sigma.webgl.edges.arrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.labels.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.hovers.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.nodes.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edges.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edges.curve.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edges.arrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edges.curvedArrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edgehovers.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edgehovers.curve.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edgehovers.arrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.edgehovers.curvedArrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/canvas/sigma.canvas.extremities.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.utils.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.nodes.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.edges.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.edges.curve.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.edges.curvedArrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.labels.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/renderers/svg/sigma.svg.hovers.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/middlewares/sigma.middlewares.rescale.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/middlewares/sigma.middlewares.copy.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/misc/sigma.misc.animation.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/misc/sigma.misc.bindEvents.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/misc/sigma.misc.bindDOMEvents.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/src/misc/sigma.misc.drawHovers.js'?>"></script>
    <!-- END SIGMA IMPORTS -->

    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.parsers.gexf/gexf-parser.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.parsers.gexf/sigma.parsers.gexf.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.animate/sigma.plugins.animate.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.layouts.forceLink/worker.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.layouts.forceLink/supervisor.js'?>"></script>

    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.parsers.json/sigma.parsers.json.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.dragNodes/sigma.plugins.dragNodes.js'?>"></script>

    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.helpers.graph/sigma.helpers.graph.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.activeState/sigma.plugins.activeState.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.select/sigma.plugins.select.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.keyboard/sigma.plugins.keyboard.js'?>"></script>

    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/settings.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.labels.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.hovers.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.cross.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.diamond.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.equilateral.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.square.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.nodes.star.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.def.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curve.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.arrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.curvedArrow.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.renderers.linkurious/canvas/sigma.canvas.edges.autoCurve.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.layouts.fruchtermanReingold/sigma.layout.fruchtermanReingold.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.locate/sigma.plugins.locate.js'?>"></script>

    <!--  ToolTip Plugin -->
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.tooltips/sigma.plugins.tooltips.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . 'https://cdnjs.cloudflare.com/ajax/libs/mustache.js/0.8.1/mustache.min.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.lasso/sigma.plugins.lasso.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/plugins/sigma.plugins.filter/sigma.plugins.filter.js'?>"></script>

    <!--  JQuery UI -->
    <script type="text/javascript" src="//code.jquery.com/ui/1.11.4/jquery-ui.js"></script>

    <script type="text/javascript" src="<?php print $module_path . '/theme/js/ajax.js'?>"></script>
    <script type="text/javascript" src="<?php print $module_path . '/theme/js/dynamic.js'?>"></script>

    <script type="text/javascript">
      var network_url  = "<?php print $module_path ?>";
    </script>

    <script type="text/javascript" src="<?php print $module_path . '/theme/js/tripal_network_viewer.js'?>"></script>
    <?php print $head ?>
  </head>
  <body class="claro">
      <?php print $page ?>
  </body>
</html>