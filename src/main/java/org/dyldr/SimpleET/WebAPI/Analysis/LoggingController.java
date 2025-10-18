package org.dyldr.SimpleET.WebAPI.Analysis;


import org.dyldr.SimpleET.DB.LoggingTable;
import org.dyldr.SimpleET.Entity.LoggingEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

@RestController
@RequestMapping("/analysis/log")
public class LoggingController {

    record LogRet<T>(int code, T data) {
    }

    @Autowired
    private LoggingTable loggingTable;

    @GetMapping("")
    public LogRet<Object> getAll(
            @RequestAttribute(value = "token") Map<String,String> token,
            @RequestParam(value = "action") String action,
            @RequestParam(value = "start", required = false, defaultValue = "") String start,
            @RequestParam(value = "end", required = false, defaultValue = "") String end
    ) {
        if (!token.get("role").equals("ADMIN")) {
            return new LogRet<>(-1, "Permission denied");
        }
        try {
            if (!start.equals("") && !end.equals("")) {
                return new LogRet<>(0, loggingTable.findAllByActionAndTimestampBetween(
                        LoggingEntity.ActionType.valueOf(action),
                        new java.util.Date(Long.parseLong(start)),
                        new java.util.Date(Long.parseLong(end))
                ));
            }
            return new LogRet<>(0, loggingTable.findAllByAction(LoggingEntity.ActionType.valueOf(action)));
        } catch (Exception e) {
            return new LogRet<>(-2, "Cannot find log");
        }
    }





}
