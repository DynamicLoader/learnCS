package org.dyldr.SimpleET.Util;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.type.CollectionType;
import com.fasterxml.jackson.databind.type.MapType;

import jakarta.persistence.AttributeConverter;
import jakarta.persistence.Converter;

import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;


@Converter
public class Cart2JsonConverter implements AttributeConverter<Map<String,Integer>, String> {

    private static final ObjectMapper mapper = new ObjectMapper();
    private static final MapType type = mapper.getTypeFactory().constructMapType(HashMap.class, String.class, Integer.class);

    @Override
    public String convertToDatabaseColumn(Map<String,Integer> attribute) {
        try {
            return mapper.writeValueAsString(attribute);
        } catch (JsonProcessingException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public Map<String,Integer> convertToEntityAttribute(String dbData) {
        try {
            return mapper.readValue(dbData, type);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
