package org.dyldr.SimpleET.Util;

import com.auth0.jwt.JWT;
import com.auth0.jwt.JWTCreator;
import com.auth0.jwt.algorithms.Algorithm;
import com.auth0.jwt.interfaces.DecodedJWT;
import jakarta.annotation.PostConstruct;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;
import org.springframework.stereotype.Component;

import java.util.Calendar;
import java.util.Map;

@Configuration
class JWTHelper{
    @Value("${auth.jwt.server_secret:SimpleETSecret}")
    private String secret;

    public String getSecret() {
        return secret;
    }
}

@Component
public class JWTUtils {


    private static String SECRET;

    @Autowired
    private JWTHelper jwtHelper;

    @PostConstruct
    public void init() {
        SECRET = jwtHelper.getSecret();
    }

    /**
     * Generate token
     *
     * @param expire Expire Time, in minutes
     * @param map    payload
     * @return token string
     */
    public static String generateToken(int expire, Map<String, String> map) {
        Calendar instance = Calendar.getInstance();
        instance.add(Calendar.SECOND, expire);

        // 创建 jwt 构造器
        JWTCreator.Builder builder = JWT.create();

        // 设置 payload
        map.forEach(builder::withClaim);

        // 设置过期时间, 生成 token
        return builder.withExpiresAt(instance.getTime()).sign(Algorithm.HMAC256(SECRET));
    }

    /**
     * verify token
     *
     * @param token token to be verified
     * @return DecodedJWT object
     */
    public static DecodedJWT verifyToken(String token) {
//        System.out.println(SECRET);
        return JWT.require(Algorithm.HMAC256(SECRET))
                .build().verify(token);
    }
}