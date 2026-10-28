package com.recsys.api.controller;

import com.recsys.api.model.Rating;

import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.web.bind.annotation.*;
import redis.clients.jedis.Jedis;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

@RestController
public class ApiController {
    private final KafkaTemplate<String, String> kafkaTemplate;
    private final Jedis jedis = new Jedis("redis", 6379);
    private final ObjectMapper mapper = new ObjectMapper();

    public ApiController(KafkaTemplate<String, String> kafkaTemplate) {
        this.kafkaTemplate = kafkaTemplate;
    }

    @PostMapping("/rate")
    public String addRating(@RequestBody Rating rating) {
        try {
            String json = mapper.writeValueAsString(rating);
            kafkaTemplate.send("ratings", json);
            return "{\"status\": \"rating received\"}";
        } catch (JsonProcessingException e) {
            return "{\"status\": \"error\"}";
        }
    }

    @GetMapping("/recommend")
    public String getRecommendations(@RequestParam("userId") int userId) {
        String cached = jedis.get("user:" + userId);
        if (cached != null)
            return cached;
        return "{\"userId\": " + userId + ", \"recommendations\": []}"; // Fallback
    }
}