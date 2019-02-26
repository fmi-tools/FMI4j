package no.ntnu.ihb.fmi4j.modeldescription.jacskon

import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import com.fasterxml.jackson.dataformat.xml.annotation.JacksonXmlProperty
import no.ntnu.ihb.fmi4j.modeldescription.SimpleType


typealias TypeDefinitionsImpl = List<SimpleTypeImpl>

/**
 * @author Lars Ivar Hatledal
 */
@JsonIgnoreProperties(ignoreUnknown = true)
data class SimpleTypeImpl (

        @JacksonXmlProperty
        override val name: String,

        @JacksonXmlProperty
        override val description: String? = null

) : SimpleType
