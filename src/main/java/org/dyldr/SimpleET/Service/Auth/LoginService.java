package org.dyldr.SimpleET.Service.Auth;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.Entity.*;
import org.dyldr.SimpleET.DB.*;
import org.dyldr.SimpleET.Util.*;

import org.springframework.stereotype.Service;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.List;

@Service
public class LoginService {

    public enum ErrorCode {
        OK,
        INVALID_USERNAME,
        INVALID_PASSWORD,
        INTERNAL_ERROR
    }

    public record srvRet(ErrorCode statusCode, UserEntity user) {
    }

    @Autowired
    private UserTable usr;

    public srvRet login(String username, String password) {
        UserEntity u;
        List<UserEntity> ul = usr.findByName(username);
        if (ul.size() != 1) {
            return new srvRet(ErrorCode.INVALID_USERNAME, null);
        }

        u = ul.get(0);
        if (!u.getPassword().equals(PasswordUtils.hashPassword(password, u.getSalt()))) {
            return new srvRet(ErrorCode.INVALID_PASSWORD, null);
        }
        return new srvRet(ErrorCode.OK, u);
    }

    @Transactional
    public srvRet register(
            String username,
            String password,
            String email,
            String role) {
        UserEntity u = new UserEntity();
        u.setName(username);
        u.setSalt(PasswordUtils.generateSalt());
        u.setPassword(PasswordUtils.hashPassword(password, u.getSalt()));
        u.setEmail(email);
        u.setRole(UserEntity.UserRole.fromString(role));
        u.setStatus(UserEntity.UserStatus.ACTIVE); // Todo
        usr.save(u);
        return new srvRet(ErrorCode.OK, u);
    }
}
