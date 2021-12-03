<xsl:stylesheet xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="code_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request exsl">
  <xsl:namespace-alias stylesheet-prefix="xxx" result-prefix="xsl"/>

  <!-- exclude prefixes in the transform RESULT
    NOTE: all these prefixes need to be declared on the xsl:stylesheet
      doc->setNamespaceRoot() for this stylesheet output
      will ensure these @xmlns:* are declared
  -->
  <xsl:param name="gs_exclude_namespace_prefixes_from_HTML" select="'#default repository gs database debug conversation html rx class xmlsecurity xmltransaction xsl object interface xsd service javascript css'"/>

  <xsl:template match="xsl:stylesheet" mode="gs_code_render_dynamic">
    <!-- DSXL is standalone: the input-node is always the entire readable system -->
    <xxx:stylesheet>
      <xsl:copy-of select="@xml:id|@version|@exclude-result-prefixes"/>
      <xsl:if test="not(@version)"><xsl:attribute name="version">1.0</xsl:attribute></xsl:if>
      <xsl:if test="not(@exclude-result-prefixes)"><xsl:attribute name="exclude-result-prefixes"><xsl:value-of select="$gs_exclude_namespace_prefixes_from_HTML"/></xsl:attribute></xsl:if>
      <xsl:copy-of select="$gs_request/gs:query-string/@*[not(self::version|self::exclude-result-prefixes)]"/>
      <xsl:attribute name="response:include-all">on</xsl:attribute>
      <xsl:attribute name="meta:xpath-to-node">~XSLStylesheet/code_render</xsl:attribute>
      
      <xsl:if test="not(xsl:namespace-alias)">
        <xsl:comment>no xsl:namespace-alias statement present. how will xsl be output?</xsl:comment>
        <debug:server-message output="no xsl:namespace-alias statement present in dynamic xsl:stylesheet [{@xml:id}]"/>
      </xsl:if>

      <xsl:if test="not(xsl:template[@mode='gs_generate_XSL'])">
        <xsl:comment>no xsl:template @mode=gs_generate_XSL present. how will xsl be generated?</xsl:comment>
        <debug:server-message output="no xsl:template @mode=gs_generate_XSL present in dynamic xsl:stylesheet [{@xml:id}]"/>
      </xsl:if>

      <xsl:comment>DXSL Dynamic XSL generated template from [<xsl:value-of select="@xml:id"/> / <xsl:value-of select="@name"/>]</xsl:comment>
      <xsl:variable name="gs_stylesheet" select="."/>
      <database:transform select="$gs_stylesheet" stylesheet="$gs_stylesheet" interface-mode="gs_generate_XSL"/>
    </xxx:stylesheet>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_code_render_direct_simple">
    <xsl:copy>
      <xsl:copy-of select="@version|@exclude-result-prefixes"/>
      <xsl:if test="not(@version)"><xsl:attribute name="version">1.0</xsl:attribute></xsl:if>
      <xsl:if test="not(@exclude-result-prefixes)"><xsl:attribute name="exclude-result-prefixes"><xsl:value-of select="$gs_exclude_namespace_prefixes_from_HTML"/></xsl:attribute></xsl:if>
      <xsl:copy-of select="$gs_request/gs:query-string/@*[not(self::version|self::exclude-result-prefixes)]"/>
      <xsl:attribute name="response:include-all">on</xsl:attribute>
      <xsl:attribute name="meta:xpath-to-node">~XSLStylesheet/code_render</xsl:attribute>

      <!--
        this xsl:output will cause a DOCTYPE in all XHTML output that includes this stylesheet
        and a META Content-type into the HEAD to state MIME Type and charset
        XML conform with the XHTML standard (HTML produces invalid XML like the <img> and <meta> tags) but produces an XML Document client side which does not have a normal HTML document object and thus all javascript fails
        HTML will produce a standard HTML Document (invalid XML) with javascript functions attached so that things like FCKEditor can work

      XML output - needs an XHTML default namespace in all the stylesheets
      <xsl:output
        method="xml"
        version="1.0"
        encoding="utf-8"
        doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
        doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
      />

      Loose HTML 4.01 (includes various depreciated attributes)
      <xsl:output
        method="html"
        version="4.01"
        encoding="utf-8"
        doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN"
        doctype-system="http://www.w3.org/TR/html4/loose.dtd"
      />

      Strict HTML 4.01
      -->
      <xsl:if test="not(xsl:output) and str:boolean(@meta:write-output, true())">
        <xsl:comment>recommended xsl:output (firefox requires a HTMLDocument object from @method to work)</xsl:comment>
        <xsl:if test="$gs_request/gs:query-string/@server-side-xslt = 'true'">
          <!-- we will be outputting HTTP headers so we do not want a doctype-* at the beginning -->
          <xsl:copy-of select="$gs_system_entities/xsl:output[@name = 'HTML without doctype']"/>
          <debug:server-message output="object:Request [{$gs_request/gs:message_type}] xsl:output server-side-xslt without @doctype-public"/>
        </xsl:if>
        <xsl:else>
          <xsl:copy-of select="$gs_system_entities/xsl:output[@name = 'HTML']"/>
          <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] xsl:output with @doctype-public"/>
        </xsl:else>
      </xsl:if>

      <!-- stylesheet contents
        currently only the xsl:include|xsl:import may be changed
        xsl:templates are controlled by a separate gs_code_render
      -->
      <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; gs_code_render [{name()}]"/>
      <xsl:apply-templates select="xsl:*" mode="gs_code_render">
        <xsl:with-param name="gs_create_meta_context_attributes" select="'none'"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_client_side_xsl_output_checks">
    <xsl:if test="@name='controller' or @name='code_render' or @name='interface_render' or @name='inheritance'">
      <debug:server-message output="xsl:stylesheet [{@xml:id}] is server-side in client-side output" type="error"/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
