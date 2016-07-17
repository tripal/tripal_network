<?php
$module_path = drupal_get_path('module', 'tripal_network');
?>

<div id="graph-container">
  <span id="layout-notification"></span>
</div>

<div id="name" style="position:absolute;left:90%;color:#202020;top:1%;"> <?php
  if (isset($_POST["submit"])) {
    if (strlen($_POST["species"]) < 8) { ?>
       <span style='font-size:30px;font-weight:200;font-family:Roboto'><?php print strtoupper($species) ?></span> <?php
    }
    else { ?>
       <span style='font-size:20px;font-weight:200;font-family:Roboto'><?php print strtoupper($species) ?></span> <?php
    }
  } ?>

  <br />
  <span style="font-size:15px;font-weight:200;font-family:Roboto;">' <?php
    if (isset($_POST["module"])) {
      print $module;
    } ?>
  </span>
</div>

<div id="tripal-network-viewer-filter-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Filters</h2>
  </div>
  <div class="toggle-content"><?php
    // Add in the filter form.
    module_load_include('inc', 'tripal_network', 'includes/tripal_network.viewer');
    $form =  drupal_get_form('tripal_network_viewer_filter_form');
    print(drupal_render($form)); ?>
  </div>
</div>

<div id="tripal-network-viewer-settings-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Display</h2>
  </div>
  <div class="toggle-content">
    <h4>Select Node</h4>
    <select id="nodelist">
      <option value="" selected>All nodes</option>
    </select>
    <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
  </div>
</div>



<div id="tripal-network-viewer-data-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Data</h2>
  </div>
  <div class="toggle-content">
    <ul>
      <li><a href="#data-panel-edge-list">Edge List</a></li>
      <li><a href="#data-panel-node-list">Node List</a></li>
      <li><a href="#data-panel-markers-list">Markers</a></li>
      <li><a href="#data-panel-functional-list">Functional</a></li>
      <li><a href="#data-panel-selected-list">Current Selection</a></li>
    </ul>
    <div id="data-panel-edge-list" class="data-panel-item">
      <table id="data-panel-edge-list-table">
        <thead>
          <tr>
            <th>Number</th>
            <th>Source </th>
            <th>Target</th>
            <th>Weight</th>
            <th>Direction</th>
            <th>Selected Traits</th>
          </tr>
        </thead>
        <tbody><?php
          if (isset($_POST["submit"])) {
            for ($i = 0; $i < $edge_count; $i++) {
              $j=$i+1;
              $src = $edges[$i]["source"];
              $source = $nodes[(int)$src];
              $trg = $edges[$i]["target"];
              $target = $nodes[(int)$trg]; ?>
              <tr>
                <td><?php print $j?></td>
                <td><?php print $source ?></td>
                <td><?php print $target ?></td>
                <td>1.5</td>
                <td>undirected</td>
                <td></td>
              </tr><?php
            }
          }
          else { ?>
            <tr><td colspan="6"><i>No Edges Selected</i></td></tr><?php
          }
          ?>
        </tbody>
      </table>
    </div>

    <div id="data-panel-node-list">
      <table>
        <thead>
          <tr>
            <th>Number</th>
            <th>Node List</th>
            <th>Functions</th>
          </tr>
        </thead>
        <tbody> <?php
          if (isset($_POST["submit"])) {
           for($i = 0; $i < $num; $i++) {
             $j = $i + 1; ?>
             <tr>
               <td><?php print $j ?></td>
               <td><?php print $nodes[$i] ?></td>
               <td>Unknown</td>
             </tr> <?php
            }
          } ?>
        </tbody>
      </table>
    </div>

    <div id="data-panel-markers-list">
    </div>

    <div id="data-panel-functional-list">
    </div>

    <div id="data-panel-selected-list">
    </div>
  </div>
</div>

<div id="info_basic">
  <span style="font-size:20px"><?php
    if (isset($_POST["submit"])){ ?>
      Nodes: <?php
    } ?>
  </span>
  <span style="font-size:25px;"><?php
    if (isset($_POST["submit"])) {
      print "  " . $num;
    }  ?>
  </span>
  <br />
  <span style="font-size:20px"> <?php
    if (isset($_POST["submit"])) { ?>
      Edges: <?php
    } ?>
  </span>
  <span style="font-size:25px;"> <?php
    if (isset($_POST["submit"])) {
      print $edge_count;
    } ?>
  </span>
</div>