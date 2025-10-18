package org.dyldr.SimpleET.WebAPI;

import com.auth0.jwt.interfaces.Claim;
import jakarta.servlet.http.Cookie;
import org.dyldr.SimpleET.Util.JWTUtils;

import com.auth0.jwt.exceptions.AlgorithmMismatchException;
import com.auth0.jwt.exceptions.IncorrectClaimException;
import com.auth0.jwt.exceptions.SignatureVerificationException;
import com.auth0.jwt.exceptions.TokenExpiredException;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.MediaType;
import org.springframework.web.servlet.HandlerInterceptor;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Map;


@Slf4j
public class JWTInterceptor implements HandlerInterceptor {

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
        Map<String, Object> responseMap = new HashMap<>();

        Cookie[] cookies = request.getCookies();
        String token = null;

        if (cookies == null) {
            responseMap.put("code", 401);
            responseMap.put("msg", "Login Required");
            String json = new ObjectMapper().writeValueAsString(responseMap);
            response.setContentType(MediaType.APPLICATION_JSON_VALUE);  // application/json
            PrintWriter writer = response.getWriter();
            writer.println(json);
            return false;
        }

        for (Cookie cookie : cookies) {
            if ("token".equals(cookie.getName())) {
                token = cookie.getValue();
                break;
            }
        }

        if(token == null) {
            responseMap.put("code", 401);
            responseMap.put("msg", "Login Required");
            String json = new ObjectMapper().writeValueAsString(responseMap);
            response.setContentType(MediaType.APPLICATION_JSON_VALUE);  // application/json
            PrintWriter writer = response.getWriter();
            writer.println(json);
            return false;
        }


        try {
             Map<String, Claim> m =  JWTUtils.verifyToken(token).getClaims();
            Map<String, String> map = new HashMap<>();
            for (Map.Entry<String, Claim> entry : m.entrySet()) {
                map.put(entry.getKey(), entry.getValue().asString());
            }
            request.setAttribute("token", map);
            return true;
        } catch (SignatureVerificationException e) {
            log.error("Invalid JWT Sign: {}", e.getMessage());
            responseMap.put("msg", "Invalid JWT Sign");
        } catch (TokenExpiredException e2) {
            log.error("Expired JWT: {}", e2.getMessage());
            responseMap.put("msg", "Expired JWT");
            throw new RuntimeException(e2);
        } catch (AlgorithmMismatchException | IncorrectClaimException e3) {
            log.error("Unsupported JWT: {}", e3.getMessage());
            responseMap.put("msg", "Unsupported JWT");
        } catch (Exception e4) {
            log.error("Invalid JWT: {}", e4.getMessage());
            responseMap.put("msg", "Invalid JWT");
        }

        responseMap.put("code", 401);
        String json = new ObjectMapper().writeValueAsString(responseMap);
        response.setContentType(MediaType.APPLICATION_JSON_VALUE);  // application/json
        PrintWriter writer = response.getWriter();
        writer.println(json);
        return false;
    }
}