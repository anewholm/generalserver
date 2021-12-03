<xsl:stylesheet response:server-side-only="true" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="xpath_attribute_to_hardlink" version="1.0" extension-element-prefixes="dyn database server debug">
  <xsl:template match="object:Request/gs:query-string|object:Request/gs:form">
    <xsl:apply-templates select="@*" mode="gs_hardlink_xpath_attributes"/>
    <!-- done here to show the hardlinking results 
      @max-depth so that we see the results of the softlinks
    -->
    <debug:server-message if="$gs_debug_url_steps" output-node="$gs_request" max-depth="5"/>
  </xsl:template>

  <xsl:template match="@*" mode="gs_hardlink_xpath_attributes">
    <!-- ALREADY created: the element that holds the hardlinks is  
      <query-string @xpath-to-things="this|that|...">
        <xpath-to-things>
          <this>...</this> (hardlink)
          <that>...</that> (hardlink)
          ...
        </xpath-to-things>
      </query-string>
      NOTE: that matches happen on local-name(), not name()
    -->
    <xsl:variable name="gs_local_name" select="local-name()"/>
    <xsl:variable name="gs_existing_holding_element" select="../*[local-name()=$gs_local_name]"/>
 
    <xsl:if test="$gs_existing_holding_element">
      <!-- just in case 1 is sent through to indicate true, e.g. data-only=1 -->
      <xsl:if test="not(. = '1')">
        <xsl:choose>
          <xsl:when test="str:ends-with(local-name(), '-id')">
            <xsl:variable name="gs_singular_referenced_node" select="id(.)"/>
            <debug:server-message if="$gs_debug_url_steps" output="@{name()} hardlinking [{count($gs_singular_referenced_node)}] @xml:id [{.}] node"/>
            <xsl:if test="$gs_singular_referenced_node">
                <database:softlink-child select="$gs_singular_referenced_node" destination="$gs_existing_holding_element"/>
            </xsl:if>
            <xsl:else>
                <debug:server-message if="$gs_debug_url_steps" output="@{name()} not found @xml:id [{.}] node"/>
            </xsl:else>
          </xsl:when>
          
          <xsl:when test="starts-with(local-name(), 'class')">
            <xsl:variable name="gs_class_xpath">
              <xsl:if test="starts-with(., '~'') or starts-with(., '/') or starts-with(., '$') or starts-with(., '!') or contains(., '(')">
                <!-- interpret as xpath -->
                <xsl:value-of select="."/>
              </xsl:if>
              <xsl:else>
                <!-- interpret as a class name -->
                <xsl:text>~</xsl:text>
                <xsl:if test="starts-with(., 'Class__')">
                  <xsl:value-of select="substring-after(., 'Class__')"/>
                </xsl:if>
                <xsl:else>
                  <xsl:value-of select="."/>
                </xsl:else>
              </xsl:else>
            </xsl:variable>
            <xsl:variable name="gs_class_node" select="dyn:evaluate($gs_class_xpath)"/>
            
            <xsl:if test="not($gs_class_node)">
              <debug:server-message output="@{name()} hardlinking of class [{$gs_class_xpath}] node failed" type="warning"/>
            </xsl:if>
            
            <xsl:else>
              <debug:server-message if="$gs_debug_url_steps" output="@{name()} hardlinking of [{count($gs_class_node)}] class [{$gs_class_xpath} = {name($gs_class_node)}] node"/>
              <debug:server-message output-node="$gs_class_node"/>
              <database:softlink-child select="$gs_class_node" destination="$gs_existing_holding_element"/>
            </xsl:else>
          </xsl:when>
          
          <xsl:when test="starts-with(local-name(), 'xpath') or starts-with(local-name(), 'data')">
            <xsl:variable name="gs_multiple_referenced_nodes" select="dyn:evaluate(.)"/>
            <debug:server-message if="$gs_debug_url_steps" output="@{name()} hardlinking of [{count($gs_multiple_referenced_nodes)}] nodes [{.} = {name($gs_multiple_referenced_nodes)}]"/>
            <database:softlink-children select="$gs_multiple_referenced_nodes" destination="$gs_existing_holding_element"/>
          </xsl:when>
        </xsl:choose>
      </xsl:if>
    </xsl:if>
    <xsl:else>
      <debug:server-message if="$gs_debug_url_steps" output="@{name()} existing holding element not found [{$gs_local_name}]"/>
    </xsl:else>
  </xsl:template>
</xsl:stylesheet>
