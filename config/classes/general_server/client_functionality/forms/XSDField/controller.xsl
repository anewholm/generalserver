<xsl:stylesheet xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns="http://www.w3.org/1999/xhtml" name="controller" controller="true" response:server-side-only="true" version="1.0" extension-element-prefixes="debug request regexp str flow database repository">
  <xsl:template match="xsd:element" mode="gs_form_to_object">
    <xsl:param name="gs_this_form"/>
    <xsl:param name="gs_class_definition"/>

    <xsl:variable name="gs_field_name" select="string(@name)"/>
    <xsl:variable name="gs_field_value" select="$gs_this_form/*[name() = $gs_field_name]"/>
    <debug:server-message output="  [{$gs_class_definition/@elements}/{$gs_field_name}] =&gt; [{$gs_field_value}]"/>

    <!-- notice that we default the namespace to GS here -->
    <xsl:element xmlns="http://general_server.org/xmlnamespaces/general_server/2006" name="{$gs_field_name}">
      <xsl:value-of select="$gs_field_value"/>
    </xsl:element>
  </xsl:template>

  <xsl:template match="xsd:attribute" mode="gs_form_to_object">
    <xsl:param name="gs_this_form"/>
    <xsl:param name="gs_class_definition"/>

    <xsl:variable name="gs_field_name" select="string(@name)"/>
    <xsl:variable name="gs_field_value" select="$gs_this_form/*[name() = $gs_field_name]"/>
    <debug:server-message output="  [{$gs_class_definition/@elements}/{$gs_field_name}] =&gt; [@{$gs_field_value}]"/>

    <!-- notice that we default the namespace to GS here -->
    <xsl:attribute xmlns="http://general_server.org/xmlnamespaces/general_server/2006" name="{$gs_field_name}">
      <xsl:value-of select="$gs_field_value"/>
    </xsl:attribute>
  </xsl:template>
</xsl:stylesheet>