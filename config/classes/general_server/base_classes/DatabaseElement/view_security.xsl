<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_security" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="securitypermissions" meta:interface-template="yes">
    <!-- @xmlsecurity:* on the associated node
      xmlsecurity:ownername="root"
      xmlsecurity:ownergroupname="grp_1"
      xmlsecurity:permissions="740"
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <!-- TODO: inheritance? -->
    <xsl:variable name="gs_xmlsecurity_permissons">
      <xsl:if test="@xmlsecurity:permissions and string-length(string(@xmlsecurity:permissions)) = 3"><xsl:value-of select="@xmlsecurity:permissions"/></xsl:if>
      <xsl:if test="not(@xmlsecurity:permissions and string-length(string(@xmlsecurity:permissions)) = 3)">777</xsl:if>
    </xsl:variable>

    <div class="{$gs_html_identifier_class}">
      <div class="gs-securitypermissions-number">
        [<xsl:value-of select="$gs_xmlsecurity_permissons"/>]
        <xsl:if test="@xmlsecurity:permissions"> /
          [<xsl:value-of select="@xmlsecurity:permissions"/>]
        </xsl:if>
        <xsl:if test="not(@xmlsecurity:permissions)">(defaulted)</xsl:if>
        <xsl:if test="@xmlsecurity:permissions and not(string-length(string(@xmlsecurity:permissions)) = 3)">(invalid)</xsl:if>
      </div>

      <table>
        <tr><th>context</th><th>read</th><th>write</th><th>execute</th></tr>

        <tr><td class="gs-context">owner</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 1, 1)"/></xsl:apply-templates>
        </tr>
        <tr><td class="gs-context">group</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 2, 1)"/></xsl:apply-templates>
        </tr>
        <tr><td class="gs-context">other</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 3, 1)"/></xsl:apply-templates>
        </tr>
      </table>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </div>
  </xsl:template>

  <xsl:template match="*" mode="gs_security_unix_permission">
    <xsl:param name="digit"/>

    <td><input class="gs-read" name="read" type="checkbox">
      <xsl:if test="floor($digit div 4) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
    <td><input class="gs-write" name="write" type="checkbox">
      <xsl:if test="floor($digit div 2) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
    <td><input class="gs-execute" name="execute" type="checkbox">
      <xsl:if test="floor($digit div 1) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
  </xsl:template>
</xsl:stylesheet>
