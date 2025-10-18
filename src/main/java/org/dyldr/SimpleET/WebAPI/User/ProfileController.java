package org.dyldr.SimpleET.WebAPI.User;

import org.dyldr.SimpleET.DB.UserTable;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestAttribute;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;

@RestController
@RequestMapping("/user")
public class ProfileController {

    @Autowired
    private UserTable userTable;

    record ProfileRet<T>(int code, T data) {
    }

    @GetMapping("")
    public ProfileRet<Object> getProfile(
            @RequestAttribute Map<String, String> token
    ) {
        try {
            if(token.get("role").equals("ADMIN"))
                return new ProfileRet<>(0, StreamSupport.stream(userTable.findAll().spliterator(), false)
                        .map(user -> {
                            // 过滤掉不需要的字段
                            user.setPassword(null);
                            user.setSalt(null);
                            return user;
                        })
                        .collect(Collectors.toList()));
            return new ProfileRet<>(-2, "Permission denied");
        } catch (Exception e) {
            return new ProfileRet<>(-1, "Cannot find user");
        }
    }
}
