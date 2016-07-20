<?php
$module_path = drupal_get_path('module', 'tripal_network');

?>
<script type="text/javascript">
  var network_data;
</script>

<div id="graph-container">
  <span id="layout-notification"></span>
</div>


<div id="tripal-network-viewer-settings-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Display</h2>
  </div>
  <div class="toggle-content">
    <div id="bg-color-picker">
      <h4>Background Color</h4>
      <div class="bg-color-picker" style="background-color: #FFFFFF">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #000044">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #666666">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #444444">&nbsp</div>
      <div class="bg-color-picker" style="background-color: #000000">&nbsp</div>
    </div>
    <div id="select-form">
      <div class="form-item">
        <label>Select Node</label>
        <select id="nodelist">
          <option value="" selected>All nodes</option>
        </select>
      </div>
      <div class="form-item">
        <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
      </div>
    </div>
  </div>
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


<div id="tripal-network-viewer-data-panel">
  <div class="toggle-header">
    <img class="toggle-icon" src="<?php print $module_path . '/theme/images/menu_icon.png'?>">
    <h2>Data</h2>
  </div>
  <div class="toggle-content">
    <ul>
      <li><a href="#data-panel-network-stats">Network</a></li>
      <li><a href="#data-panel-edge-list">Edges</a></li>
      <li><a href="#data-panel-node-list">Nodes</a></li>
      <li><a href="#data-panel-markers-list">Markers</a></li>
      <li><a href="#data-panel-functional-list">Functional</a></li>
    </ul>
    <div id="data-panel-network-stats" class="data-panel-item">
    Network stats go here.
    </div>
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
            <th>Node Name</th>
            <th>Function Annotations</th>
          </tr>
        </thead>
        <tbody> <?php
          if (isset($_POST["submit"])) {
           for($i = 0; $i < $num; $i++) {
             $j = $i + 1; ?>
             <tr>
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

  </div>
</div>