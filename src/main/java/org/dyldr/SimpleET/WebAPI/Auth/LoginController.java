package org.dyldr.SimpleET.WebAPI.Auth;

import jakarta.servlet.http.Cookie;
import jakarta.servlet.http.HttpServletResponse;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.dyldr.SimpleET.Service.Auth.LoginService;
import org.dyldr.SimpleET.Util.JWTUtils;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.*;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.Calendar;
import java.util.Date;
import java.util.Map;


@RestController
@RequestMapping("/auth")
public class LoginController {

    record LoginRet(long code, String... msg) {
    }

    @Value("${auth.token_expire_time:3600}")
    private int TOKEN_EXPIRE_TIME;

    @Value("${auth.super_secret:super_secret}")
    private String SUPER_SECRET;

    @Autowired
    LoginService loginService;

    @GetMapping("login")
    public LoginRet check(
            @CookieValue(value = "token", defaultValue = "") String token
    ) {
        try {
            JWTUtils.verifyToken(token);
            return new LoginRet(0, "Already logged in");
        } catch (Exception e) {
            return new LoginRet(1, "Invalid token");
        }
    }

    @PostMapping("login")
    public LoginRet login(@RequestParam(value = "username") String username,
                          @RequestParam(value = "password") String password,
                          HttpServletResponse response
    ) {
        try {
            LoginService.srvRet ret = loginService.login(username, password);
            if (ret.statusCode() == LoginService.ErrorCode.OK) {
                Cookie c = new Cookie("token", JWTUtils.generateToken(TOKEN_EXPIRE_TIME, Map.of(
                        "uid", ret.user().getId(),
                        "role", ret.user().getRole().toString())
                ));
                c.setMaxAge(TOKEN_EXPIRE_TIME);
                c.setPath("/");
//                c.setHttpOnly(true);
                response.addCookie(c);
                return new LoginRet(0, "Login successful", ret.user().getId(), ret.user().getRole().toString());
            } else {
                return new LoginRet(1, "Login failed! Invalid username or password");
            }
        } catch (Exception e) {
            return new LoginRet(2, "Login failed! Internal error");
        }
    }

    @PostMapping("logout")
    public LoginRet logout(HttpServletResponse response) {
        Cookie cookie = new Cookie("token", null);
        cookie.setMaxAge(0);
        cookie.setPath("/");
        response.addCookie(cookie);
        return new LoginRet(0, "Logout successful");
    }

    @PostMapping("register")
    public LoginRet register(@RequestParam(value = "username") String username,
                             @RequestParam(value = "password") String password,
                             @RequestParam(value = "email") String email,
                             @RequestParam(value = "role", defaultValue = "CUSTOMER") String role,
                             @RequestParam(value = "SuperSecret", defaultValue = "") String SuperSecret
    ) {
        if (UserEntity.UserRole.fromString(role) == UserEntity.UserRole.ADMIN && !SuperSecret.equals(SUPER_SECRET)) {
            return new LoginRet(2, "Registration failed! Invalid SuperSecret");
        }
        try {
            LoginService.srvRet ret = loginService.register(username, password, email, role);
            return new LoginRet(0, "Registration successful");
        } catch (Exception e) {
            return new LoginRet(1, "Registration failed! Invalid data or user already exists");
        }
    }
}
