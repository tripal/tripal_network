<?php

echo '<div id="graph-container"><span id="layout-notification"></span></div>
  <div id="tripal">Tripal.</div>
<div id="name" style="position:absolute;left:90%;color:#202020;top:1%;">';
 
   if(isset($_POST["submit"]))
   {
     if(strlen($_POST["species"])<8)
     {
       echo "<span style='font-size:30px;font-weight:200;font-family:Roboto'>".strtoupper($species)."</span>";
     }
     else
     {
       echo "<span style='font-size:20px;font-weight:200;font-family:Roboto'>".strtoupper($species)."</span>";
     }
     
   }
 
   
  

   echo '<br />
   <span style="font-size:15px;font-weight:200;font-family:Roboto;">';if(isset($_POST["module"])){echo $module; } echo '</span>
</div>
<div id="filter">
  <button class="pure-button pure-button-primary">Filter</button>
</div>

  <form role="form" id="dataform" method="post" action="force.php" style="background-color:rgba(255,255,255,0.7);width:200px;padding:1.6%;border:0.1px solid #C0C0C0;border-radius:3px;font-weight:300;color:white;position:absolute;top:20%;left:85%;">

   <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Species</label>

     

    <select id="module" class="styled-select slate" name="species" style="color:#202020;;border:1px solid #C0C0C0;width:100%;" onchange="loadDoc(this.value)">
                    <option>Select Species</option>
                    <option>rice</option>
                    <option>arabidopsis</option>
                    <option>maize</option>
                    
                
    </select>
  </div>
  <div class="form-group">
    <label for="pwd" style="font-weight:300;color:black;font-family:Roboto;">Module</label>

     
 
    <select id="modules" name="module" style="color:#202020;border:1px solid #C0C0C0;width:100%;">
                    


                
    </select>
  </div>
 
  <input type="submit" name="submit" value="Get graph" class="pure-button pure-button-primary"/>
</form>

 




<div id="dataset" style="" class="ui-widget-content">
<ul class="nav nav-tabs" style="color:#C0C0C0;">
  <li class="active"><a data-toggle="tab" href="#home" style="text-decoration:none;color:#A0A0A0;font-weight:300;">Edge List</a></li>
  <li><a data-toggle="tab" href="#menu1" style="color:#A0A0A0;font-weight:300;">Node List</a></li>
  <li><a data-toggle="tab" href="#menu2" style="color:#A0A0A0;font-weight:300;">Markers</a></li>
    <li><a data-toggle="tab" href="#menu3" style="color:#A0A0A0;font-weight:300;">Functions</a></li>
    <li><a data-toggle="tab" href="#menu4" style="color:#A0A0A0;font-weight:300;">Current Selection</a></li>
    


</ul>



<div class="tab-content" style="">
  <div id="home" class="tab-pane fade in active">
        <div class="table1" style="transition:all 0.6s ease;">



          
            <table style="width:100%">
                <tr>';
                   
                  if(isset($_POST["submit"]))
                  {

                   echo"<th>Number</th>
                   <th>Source </th>
                   <th>Target</th>
                   <th>Weight</th>
                   <th>Direction</th>
                   <th>Selected Traits</th>";
                 }


                echo   
                '</tr>';

                
                   if(isset($_POST["submit"]))
                   {
                     for($i=0;$i<$edge_count;$i++)
                     {
                        $j=$i+1;
                        $src = $edges[$i]["source"];
                        $source = $nodes[(int)$src];
                        $trg = $edges[$i]["target"];
                        $target = $nodes[(int)$trg];
                        echo "<tr><td>".$j."</td><td>".$source."</td><td>".$target."</td><td>1.5</td><td>undirected</td><td></td></tr>";
                     }
                   }
 
 
                
            echo '</table>
            
        </div>

  </div>
  <div id="menu1" class="tab-pane fade">
    <div class="table1">
            <table style="width:100%;">
               <tr>';
                
                 if(isset($_POST["submit"]))
                 { 
                 echo  "<th>Number</th>
                        <th>Node List</th>
                        <th>Functions </th>";
                }
                
               echo '</tr>';
         
              
                if(isset($_POST["submit"])){
                for($i=0;$i< $num;$i++)
                {
                  $j = $i+1;
                  echo "<tr><td>".$j."</td><td>".$nodes[$i]."</td><td>Unknown</td></tr>";
                }

              }
  
              





          echo   '</table>
    </div>
  </div>


  <div id="menu2" class="tab-pane fade" style="">
    <div class="table1">
    </div>
  </div>

  <div id="menu3" class="tab-pane fade" style="">
    <div class="table1">
 
        </div>
  </div>

   <div id="menu4" class="tab-pane fade" style="">
    <div class="table1">
          <div id="current_data">
             

          </div>
        </div>
  </div>




</div>



</div>


  <button id="get_table" class="pure-button pure-button-primary">Get Data</button>



 <div id="control-pane">
    <h2 class="underline">locate</h2>

    <div>
      <h4>Select Node</h4>
      <select id="nodelist">
        <option value="" selected>All nodes</option>
      </select>
    </div>
   
    <span class="line"></span>
    <div>
      <button id="reset-btn" class="pure-button pure-button-primary">Reset view</button>
    </div>
  </div>

 



<div id="info_basic" style="position:absolute;top:90%;left:90%;font-family:Roboto;background-color:rgba(255,255,255,0.7);">
  <span style="font-size:20px">'; if(isset($_POST["submit"])){echo "Nodes:"; } echo '</span> <span style="font-size:25px;">';  if(isset($_POST["submit"])){ echo "  ".$num; } echo '</span><br />';
  echo '<span style="font-size:20px">';if(isset($_POST["submit"])){echo "Edges:"; } echo '</span> <span style="font-size:25px;">';if(isset($_POST["submit"])){echo "  ".$edge_count; }  

  echo  '</span>
<div>


</div>



</body>'; 
?>