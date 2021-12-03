<xsl:stylesheet response:server-side-only="yes" xmlns:exsl="http://exslt.org/common" xmlns:flow="http://exslt.org/flow" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:session="http://general_server.org/xmlnamespaces/session/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:css="http://general_server.org/xmlnamespaces/css/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" extension-element-prefixes="dyn debug conversation flow session database" name="help" version="1.0">
  <xsl:template match="xsl:stylesheet" mode="gs_API_help">
    <!-- SERVER SIDE -->
    <html>
      <head>
        <link rel="shortcut icon" href="http://general-resources-server.localhost/resources/shared/favicon_blue.ico"></link>
        <link rel="icon" href="http://general-resources-server.localhost/resources/shared/favicon_blue.ico" type="image/x-icon"></link>
        <style>
          body {
            font-family: Verdana;
            font-size: 0.8em;
            color: #222;
          }
          h3 {
            float:right;
            margin:0px;
          }
          label {
            cursor: pointer;
          }
          label:hover {
            color: #000;
            text-shadow: 2px 2px 20px #d0d0d0;
          }
          .trace-all {
            font-weight: bold;
          }
          input.big {
            width: 400px;
          }
          #configuration-flags {
            border-collapse: collapse;
            font-size: 0.7em;
          }
          #main table, h3 {
            border-top: 1px solid #d0d0d0;
          }
          #main table.first, h3.first {
            border: none;
          }
          td {
            vertical-align: top;
          }
          #configuration-flags td {
            margin: 0px;
            padding:0px;
            vertical-align: middle;
          }
          .submit {
            position: fixed;
            top: 5px;
            right: 5px;
            height: 50px;
            box-shadow: 2px 2px 20px #888888;
            color:#fff;
            text-shadow: 1px 1px #000;
            background: linear-gradient(#ffffff, #ff8888);
          }
          .submit:hover {
            background: linear-gradient(#ff8888, #ffffff);
          }
          .help {
            font-size: 0.7em;
            color: #444;
          }
          .help:before {
            content: '(';
          }
          .help:after {
            content: ')';
          }
          #servercommand_iframe {display:none;}
          #server-commands li {
            display:inline;
            margin-right:4px;
          }
          #server-commands form {display:inline;}
          #server-commands li input {
            box-shadow: 2px 2px 20px #888888;
          }
        </style>
      </head>

      <body>
        <div></div>
        <ul id="server-commands">
          <li><form method="POST" target="servercommand_iframe" action="/api/server/help"><input type="submit" value="help"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/quit"><input type="submit" value="quit"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/reload"><input type="submit" value="reload"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/commit"><input type="submit" value="commit"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/commit full"><input type="submit" value="commit full"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/rollback"><input type="submit" value="rollback"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/p server"><input type="submit" value="p server"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/check"><input type="submit" value="check"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/p library"><input type="submit" value="p library"/></form></li>
          <li><form method="POST" target="servercommand_iframe" action="/api/server/xshell"><input type="submit" value="xshell"/></form></li>
        </ul>
          
        <form action="/api/help" onsubmit="this.setAttribute('action', '/api/' + document.getElementById('data').value)">
          <input class="submit" type="submit"/>
          <table id="main"><tr>
            <td>
              <h3 class="first">data selection</h3>
              <table class="first">
                <tr><td>data*</td><td><input id="data" class="big"/></td><td class="help">xpath, required</td></tr>
                <tr><td>login</td><td><select name="username">
                    <option value="">anonymous</option>
                    <xsl:apply-templates select="/*/repository:users/*/@name" mode="gs_options"/>
                  </select>
                  <input name="password" value="test"/>            
                  <input name="session-login" type="checkbox" checked="1" id="session-login"/><label for="session-login">persistent</label>
                </td>
                <td class="help"><xsl:value-of select="conversation:user-node()/@name"/></td>             
                </tr>
                <tr><td>node-mask</td><td><input class="big" name="node-mask" value="."/></td><td class="help">advised, e.g. node-mask=. default: class-xschema-node-mask(.), false() turns node-mask off. </td></tr>
                <tr><td>condition</td><td><input class="big" name="condition"/></td><td class="help">conditional return of the data: xpath</td></tr>
                <tr><td>evaluate</td><td><select name="evaluate">
                  <option>no</option>
                  <option>yes</option>
                </select></td></tr>
                <tr><td>database</td><td><select name="database">
                  <option value="">current</option>
                  <xsl:apply-templates select="/*/repository:databases/*/@name" mode="gs_options"/>
                </select></td><td class="help">config|&lt;database name&gt;, default: current domain database</td></tr>
                <tr><td>hardlink-policy</td><td><select name="hardlink-policy">
                  <option>suppress-exact-repeat</option>
                  <option>suppress-recursive</option>
                  <option>suppress-repeat</option>
                  <option>suppress-all</option>
                  <option>include-all</option>
                  <option>disable</option>
                </select></td></tr>
                <tr><td>controller</td><td><select name="controller">
                  <xsl:apply-templates select=".//xsl:choose[@interface:name='controller']/xsl:*[@interface:option]/@interface:option" mode="gs_options">
                    <xsl:sort select="../@interface:default" order="descending"/>
                    <xsl:sort order="descending"/>
                  </xsl:apply-templates>
                </select></td></tr>
              </table>

              <h3>data rendering</h3>
              <table>
                <tr><td>hardlink-output</td><td><select name="hardlink-output">
                  <option>follow</option>
                  <option>placeholder</option>
                  <option>ignore</option>
                </select></td></tr>
                <tr><td>database-query-output (softlinks)</td><td><select name="database-query-output">
                  <option>follow</option>
                  <option>placeholder</option>
                  <option>ignore</option>
                  <option>demand</option>
                </select></td></tr>
                <tr><td>interface-render-output (plugins)</td><td><select name="interface-render-output">
                  <option value="placeholder">placeholder (includes direct text() output)</option>
                  <option value="follow">follow (includes normalize-space())</option>
                  <option>ignore</option>
                </select></td></tr>
              </table>

              <h3>transform options</h3>
              <table>
                <tr><td>transform</td><td><input name="transform"/></td><td class="help">xpath to an xsl:stylesheet</td></tr>
                <tr><td>interface-mode    </td><td><select name="interface-mode">
                  <option value="">default (none)</option>
                  <option value="">-------- (system transforms)</option>
                  <xsl:apply-templates select="database:distinct(/config/system_transforms//xsl:stylesheet/xsl:template[str:boolean(@meta:interface-template)], '@mode')/@mode" mode="gs_options"/>
                  <option value="">-------- (class transforms)</option>
                  <xsl:apply-templates select="database:distinct(/config/classes/top::class:*/xsl:stylesheet/xsl:template[str:boolean(@meta:interface-template)], '@mode')/@mode" mode="gs_options"/>
                </select></td><td class="help">@mode attribute for the transform</td></tr>
              </table>

              <h3>output options</h3>
              <table>
                <tr><td>data-container-name</td><td><input name="data-container-name"/></td><td class="help">top level element name</td></tr>
                <tr><td>schema   </td><td><select name="schema">
                  <xsl:apply-templates select=".//xsl:choose[@interface:name='schema']/xsl:*[@interface:option]/@interface:option" mode="gs_options">
                    <xsl:sort select="../@interface:default" order="descending"/>
                    <xsl:sort order="descending"/>
                  </xsl:apply-templates>
                </select></td></tr>
                <tr><td>create-meta-context-attributes </td><td><select name="create-meta-context-attributes">
                  <option>all</option>
                  <option>class-only</option>
                  <option>none</option>
                </select></td></tr>
                <tr><td>copy-query-attributes </td><td><select name="copy-query-attributes">
                  <option>yes</option>
                  <option>no</option>
                </select></td><td class="help">TOP LEVEL output nodes only</td></tr>
                <tr><td>suppress-xmlid   </td><td><select name="suppress-xmlid">
                  <option>no</option>
                  <option>yes</option>
                </select></td></tr>
              </table>

              <h3>debug options</h3>
              <table>
                <tr><td>trace    </td><td><input name="trace"/></td><td class="help">copy in XSLT_* flags here for render specific trace</td></tr>
                <tr><td>debug    </td><td><input name="debug"/></td><td class="help">=LOG_DATA_RENDER flag</td></tr>
                <tr><td>optional</td><td><select name="optional">
                  <option value="no">required</option>
                  <option value="yes">optional</option>
                </select></td><td class="help">suppress warnings if the result is empty. default: no</td></tr>
                <tr><td>repository-name   </td><td><input name="repository-name"/></td></tr>
              </table>
            </td><td>
              <table class="first" id="configuration-flags">
                <tr><td><input name="configuration-flags" id="LOG_REQUEST_DETAIL" type="checkbox" value="LOG_REQUEST_DETAIL"/></td><td><label for="LOG_REQUEST_DETAIL">LOG_REQUEST_DETAIL</label></td></tr>
                <tr><td><input name="configuration-flags" id="LOG_DATA_RENDER" type="checkbox" value="LOG_DATA_RENDER"/></td><td><label for="LOG_DATA_RENDER">LOG_DATA_RENDER</label></td></tr>
                <tr><td><input name="configuration-flags" id="FORCE_SERVER_XSLT" type="checkbox" value="FORCE_SERVER_XSLT"/></td><td><label for="FORCE_SERVER_XSLT">FORCE_SERVER_XSLT</label></td></tr>
                <tr><td colspan="10"> </td></tr>
                
                <tr><td><input name="configuration-flags" id="XML_DEBUG_ALL" type="checkbox" value="XML_DEBUG_ALL"/></td><td><label for="XML_DEBUG_ALL" class="trace-all">XML_DEBUG_ALL</label></td></tr>
                <tr><td><input name="configuration-flags" id="XML_DEBUG_XPATH_EXPR" type="checkbox" value="XML_DEBUG_XPATH_EXPR"/></td><td><label for="XML_DEBUG_XPATH_EXPR">XML_DEBUG_XPATH_EXPR</label></td><td class="help">single expressions within steps</td></tr>
                <tr><td><input name="configuration-flags" id="XML_DEBUG_XPATH_STEP" type="checkbox" value="XML_DEBUG_XPATH_STEP"/></td><td><label for="XML_DEBUG_XPATH_STEP">XML_DEBUG_XPATH_STEP</label></td><td class="help">step / axis and results</td></tr>
                <tr><td><input name="configuration-flags" id="XML_DEBUG_PARENT_ROUTE" type="checkbox" value="XML_DEBUG_PARENT_ROUTE"/></td><td><label for="XML_DEBUG_PARENT_ROUTE">XML_DEBUG_PARENT_ROUTE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XML_DEBUG_XPATH_EVAL_COUNTS" type="checkbox" value="XML_DEBUG_XPATH_EVAL_COUNTS"/></td><td><label for="XML_DEBUG_XPATH_EVAL_COUNTS">XML_DEBUG_XPATH_EVAL_COUNTS</label></td></tr>
                <tr><td colspan="10"> </td></tr>

                <tr><td><input name="configuration-flags" id="XSLT_TRACE_ALL" type="checkbox" value="XSLT_TRACE_ALL"/></td><td><label for="XSLT_TRACE_ALL" class="trace-all">XSLT_TRACE_ALL</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_PROCESS_NODE" type="checkbox" value="XSLT_TRACE_PROCESS_NODE"/></td><td><label for="XSLT_TRACE_PROCESS_NODE">XSLT_TRACE_PROCESS_NODE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_APPLY_TEMPLATE" type="checkbox" value="XSLT_TRACE_APPLY_TEMPLATE"/></td><td><label for="XSLT_TRACE_APPLY_TEMPLATE">XSLT_TRACE_APPLY_TEMPLATE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_APPLY_TEMPLATES" type="checkbox" value="XSLT_TRACE_APPLY_TEMPLATES"/></td><td><label for="XSLT_TRACE_APPLY_TEMPLATES">XSLT_TRACE_APPLY_TEMPLATES</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_CALL_TEMPLATE" type="checkbox" value="XSLT_TRACE_CALL_TEMPLATE"/></td><td><label for="XSLT_TRACE_CALL_TEMPLATE">XSLT_TRACE_CALL_TEMPLATE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_TEMPLATES" type="checkbox" value="XSLT_TRACE_TEMPLATES"/></td><td><label for="XSLT_TRACE_TEMPLATES">XSLT_TRACE_TEMPLATES</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_VARIABLES" type="checkbox" value="XSLT_TRACE_VARIABLES"/></td><td><label for="XSLT_TRACE_VARIABLES">XSLT_TRACE_VARIABLES</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_VARIABLE" type="checkbox" value="XSLT_TRACE_VARIABLE"/></td><td><label for="XSLT_TRACE_VARIABLE">XSLT_TRACE_VARIABLE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_COPY_TEXT" type="checkbox" value="XSLT_TRACE_COPY_TEXT"/></td><td><label for="XSLT_TRACE_COPY_TEXT">XSLT_TRACE_COPY_TEXT</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_COPY" type="checkbox" value="XSLT_TRACE_COPY"/></td><td><label for="XSLT_TRACE_COPY">XSLT_TRACE_COPY</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_COMMENT" type="checkbox" value="XSLT_TRACE_COMMENT"/></td><td><label for="XSLT_TRACE_COMMENT">XSLT_TRACE_COMMENT</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_PI" type="checkbox" value="XSLT_TRACE_PI"/></td><td><label for="XSLT_TRACE_PI">XSLT_TRACE_PI</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_COPY_OF" type="checkbox" value="XSLT_TRACE_COPY_OF"/></td><td><label for="XSLT_TRACE_COPY_OF">XSLT_TRACE_COPY_OF</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_VALUE_OF" type="checkbox" value="XSLT_TRACE_VALUE_OF"/></td><td><label for="XSLT_TRACE_VALUE_OF">XSLT_TRACE_VALUE_OF</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_CHOOSE" type="checkbox" value="XSLT_TRACE_CHOOSE"/></td><td><label for="XSLT_TRACE_CHOOSE">XSLT_TRACE_CHOOSE</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_IF" type="checkbox" value="XSLT_TRACE_IF"/></td><td><label for="XSLT_TRACE_IF">XSLT_TRACE_IF</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_FOR_EACH" type="checkbox" value="XSLT_TRACE_FOR_EACH"/></td><td><label for="XSLT_TRACE_FOR_EACH">XSLT_TRACE_FOR_EACH</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_STRIP_SPACES" type="checkbox" value="XSLT_TRACE_STRIP_SPACES"/></td><td><label for="XSLT_TRACE_STRIP_SPACES">XSLT_TRACE_STRIP_SPACES</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_KEYS" type="checkbox" value="XSLT_TRACE_KEYS"/></td><td><label for="XSLT_TRACE_KEYS">XSLT_TRACE_KEYS</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_FUNCTION" type="checkbox" value="XSLT_TRACE_FUNCTION"/></td><td><label for="XSLT_TRACE_FUNCTION">XSLT_TRACE_FUNCTION</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_PARSING" type="checkbox" value="XSLT_TRACE_PARSING"/></td><td><label for="XSLT_TRACE_PARSING">XSLT_TRACE_PARSING</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_BLANKS" type="checkbox" value="XSLT_TRACE_BLANKS"/></td><td><label for="XSLT_TRACE_BLANKS">XSLT_TRACE_BLANKS</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_PROCESS" type="checkbox" value="XSLT_TRACE_PROCESS"/></td><td><label for="XSLT_TRACE_PROCESS">XSLT_TRACE_PROCESS</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_EXTENSIONS" type="checkbox" value="XSLT_TRACE_EXTENSIONS"/></td><td><label for="XSLT_TRACE_EXTENSIONS">XSLT_TRACE_EXTENSIONS</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_ATTRIBUTES" type="checkbox" value="XSLT_TRACE_ATTRIBUTES"/></td><td><label for="XSLT_TRACE_ATTRIBUTES">XSLT_TRACE_ATTRIBUTES</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_EXTRA" type="checkbox" value="XSLT_TRACE_EXTRA"/></td><td><label for="XSLT_TRACE_EXTRA">XSLT_TRACE_EXTRA</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_AVT" type="checkbox" value="XSLT_TRACE_AVT"/></td><td><label for="XSLT_TRACE_AVT">XSLT_TRACE_AVT</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_PATTERN" type="checkbox" value="XSLT_TRACE_PATTERN"/></td><td><label for="XSLT_TRACE_PATTERN">XSLT_TRACE_PATTERN</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_MOST" type="checkbox" value="XSLT_TRACE_MOST"/></td><td><label for="XSLT_TRACE_MOST">XSLT_TRACE_MOST</label></td></tr>
                <tr><td><input name="configuration-flags" id="XSLT_TRACE_NONE" type="checkbox" value="XSLT_TRACE_NONE"/></td><td><label for="XSLT_TRACE_NONE">XSLT_TRACE_NONE</label></td></tr>
              </table>
            </td>
          </tr></table>

          <div>* - required</div>
        </form>
        <iframe id="servercommand_iframe" name="servercommand_iframe"/>
      </body>
    </html>
  </xsl:template>
  
  <xsl:template match="@*" mode="gs_options">
    <xsl:param name="gs_selected"/>
    <xsl:param name="gs_suffix"/>
    
    <option>
      <xsl:if test="$gs_selected = ."><xsl:attribute name="selected">1</xsl:attribute></xsl:if>
      <xsl:value-of select="."/>
      <xsl:value-of select="$gs_suffix"/>
    </option>
  </xsl:template>
</xsl:stylesheet>
