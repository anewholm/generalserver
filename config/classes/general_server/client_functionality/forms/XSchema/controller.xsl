<xsl:stylesheet xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns="http://www.w3.org/1999/xhtml" name="controller" controller="true" response:server-side-only="true" version="1.0" extension-element-prefixes="debug request regexp str flow database repository">
  <xsl:namespace-alias result-prefix="xsl" stylesheet-prefix="xxx"/>

  <!-- TODO: included for gs_form_to_object but maybe should be a separate XSLStylesheet with 2 includes? -->
  <xsl:include xpath="~XSDField/controller"/>

  <!-- ####################################### html client input form converter ####################################### -->
  <xsl:template match="*" mode="gs_form_to_object">
    <debug:server-message output="gs_form_to_object only works with &lt;gs:form&gt;" type="warning"/>
  </xsl:template>
  
  <xsl:template match="gs:form" mode="gs_form_to_object">
    <xsl:param name="gs_element" select="string(meta:element)"/>
    
    <!-- TODO: need to process the xsd:extension also here -->
    <xsl:variable name="gs_class_definition" select="database:class(@meta:class)"/>

    <xsl:if test="not($gs_class_definition)">
      <debug:server-message output="@meta:class [{@meta:class}] required for gs_form_to_object" type="warning"/>
    </xsl:if>
    <xsl:else>
      <xsl:if test="string($gs_class_definition/@elements)">
        <debug:server-message output="[{name($gs_class_definition)}] =&gt; [{$gs_class_definition/@namespace-prefix}:{$gs_class_definition/@elements}]"/>
        <xsl:variable name="gs_name">
          <xsl:value-of select="$gs_class_definition/@namespace-prefix"/>
          <xsl:text>:</xsl:text>
          <xsl:value-of select="$gs_element"/>
          <xsl:if test="not($gs_element)"><xsl:value-of select="str:substring-before($gs_class_definition/@elements, ' ')"/></xsl:if>
        </xsl:variable>
        
        <xsl:element name="{$gs_name}">
          <xsl:apply-templates select="$gs_class_definition/xsd:schema[not(@name)]/xsd:complexType/xsd:sequence/xsd:*" mode="gs_form_to_object">
            <xsl:with-param name="gs_this_form" select="."/>
            <xsl:with-param name="gs_class_definition" select="$gs_class_definition"/>
          </xsl:apply-templates>
        </xsl:element>
      </xsl:if>
      <xsl:else>
        <debug:server-message output="@elements missing on object definition for [{name($gs_class_definition)}]" type="warning"/>
      </xsl:else>
    </xsl:else>
  </xsl:template>

  <xsl:template match="xsd:*" mode="gs_form_to_object">
    <xsl:param name="gs_this_form"/>
    <xsl:param name="gs_class_definition"/>
    <debug:server-message type="warning" output="  [{local-name($gs_class_definition)}/{name()}] not understood in gs_form_to_object"/>
  </xsl:template>

  <xsl:template match="xsd:annotation" mode="gs_form_to_object"/>

  <!-- this xsd:element|xsd:attribute part is in XSDField object
    xsl:template match="xsd:element" mode="gs_form_to_object">
  </xsl:template -->
</xsl:stylesheet>
