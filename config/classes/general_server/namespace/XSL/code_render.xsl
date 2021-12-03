<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="code_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:namespace-alias stylesheet-prefix="xxx" result-prefix="xsl"/>

  <!-- ################################################ gs_client_side_xsl_output_checks
    complement to the client-side warnings output
    TODO: here we can use reg-ex for the checks and be more thorough
  -->
  <xsl:template match="*" mode="gs_client_side_xsl_output_checks">
    <xsl:apply-templates select="*" mode="gs_client_side_xsl_output_checks"/>
  </xsl:template>

  <xsl:template match="html:*" mode="gs_client_side_xsl_output_checks">
    <xsl:apply-templates select="@*[contains(.,'{')]" mode="gs_client_side_xsl_output_checks"/>
  </xsl:template>

  <xsl:template match="@*" mode="gs_client_side_xsl_output_checks">
    <!-- TODO: regex checking -->
    <xsl:if test="contains(., 'database:') or contains(., 'str:')">
      <debug:server-message type="error" output="attribute dynamic value @{name()}={.} is not advised: it will cause FireFox to critically fail and Chrome will silently not compile it"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsl:value-of" mode="gs_client_side_xsl_output_checks">
    <!-- TODO: regex checking -->
    <xsl:if test="contains(@select, 'database:') or contains(@select, 'str:')">
      <debug:server-message type="error" output="value-of &lt;xsl:value-of @select={@select} /&gt; is not advised: it will cause FireFox to critically fail and Chrome will silently not compile it"/>
    </xsl:if>
  </xsl:template>

  <!-- ################################################ gs_code_render
    general commands not handled by specific classes
  -->
  <xsl:template match="@*" mode="gs_code_render">
    <xsl:copy/>
  </xsl:template>
  
  <xsl:template match="@xsl:*" mode="gs_code_render">
    <!-- @xsl:* attributes are never respected on the client -->
  </xsl:template>
  
  <xsl:template match="*" mode="gs_code_render">
    <!-- gs_code_render catch all literal elements 
      e.g. html:div
      TODO: stamp @meta:xpath-to-node on literals so that they can be found
    -->
    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_code_render"/>
      <xsl:apply-templates select="child::node()" mode="gs_code_render"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="*[@xsl:if]" mode="gs_code_render">
    <!-- @xsl:if 
      similar to Angular @ng:if
      e.g. <html:div @xsl:if="$usethisone" 
    -->
    <xxx:if test="{@xsl:if}">
      <xsl:copy>
        <xsl:apply-templates select="@*" mode="gs_code_render"/>
        <xsl:apply-templates select="child::node()" mode="gs_code_render"/>
      </xsl:copy>
    </xxx:if>
  </xsl:template>

  <xsl:template match="*[@xsl:when]" mode="gs_code_render">
    <!-- @xsl:when
      similar to Angular @ng:if
      e.g. <xsl:choose>
        <html:div @xsl:when="$usethisone" />
      </xsl:choose>
    -->
    <xxx:when test="{@xsl:when}">
      <xsl:copy>
        <xsl:apply-templates select="@*" mode="gs_code_render"/>
        <xsl:apply-templates select="child::node()" mode="gs_code_render"/>
      </xsl:copy>
    </xxx:when>
  </xsl:template>

  <xsl:template match="xsl:*" mode="gs_code_render">
    <!-- gs_code_render catch all xsl elements -->
    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_code_render"/>
      <xsl:apply-templates select="child::node()" mode="gs_code_render"/>
    </xsl:copy>
  </xsl:template>

  <!-- xsl:template match="@xmlsecurity:*" mode="gs_code_render"/ -->
  <xsl:template match="*[@response:server-side-only='true']|@object:cpp-component|@repository:*" mode="gs_code_render"/>

  <!-- comment output: the built-in comment template ignores them -->
  <!-- xsl:template match="comment()" mode="gs_code_render">
    <xsl:copy-of select="."/>
  </xsl:template -->

  <!-- the built-in text() handler just outputs text content -->
  <!-- xsl:template match="text()" mode="gs_code_render">
    <xsl:value-of select="str:dynamic(.)"/>
  </xsl:template -->

  <!-- the built-in ignores processing-instruction() -->
  <!-- xsl:template match="processing-instruction()" mode="gs_code_render"/ -->
</xsl:stylesheet>