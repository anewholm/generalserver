<xsl:stylesheet xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="controller" controller="true" response:server-side-only="true" version="1.0" extension-element-prefixes="debug database">
  <xsl:template match="object:Person" mode="changename">
    <xsl:param name="name_thing"/>
    <xsl:variable name="gs_context_node" select="."/>
    
    <debug:server-message output="object:Person/changename: database:set-attribute {local-name($gs_context_node)}/@test =&gt; '{$name_thing}'"/>
    <database:set-attribute select="$gs_context_node" name="test" value="{$name_thing}"/>
  </xsl:template>

  <xsl:template match="object:Person" mode="resetname">
    <xsl:variable name="gs_context_node" select="."/>
    <debug:server-message output="object:Person/resetname: database:set-attribute {local-name($gs_context_node)}/@test =&gt; 'script'"/>
    <database:set-attribute select="$gs_context_node" name="test" value="{@test}."/>
  </xsl:template>

  <xsl:template match="object:Person[@type='racist']" mode="testies">
    <!-- TODO: does this @match="object:Person[@type='racist'] expose? -->
    <weeeee/>
  </xsl:template>
</xsl:stylesheet>
