package org.dyldr.SimpleET.Entity;

import jakarta.persistence.*;
import lombok.Data;

import java.util.Date;

@Entity
@Data
public class LoggingEntity {

    public enum ActionType {
        CREATE,
        UPDATE,
        DELETE,
        LOGIN,
        LOGOUT,
        PURCHASE,
        VISIT
    }

    @Id
    @GeneratedValue(strategy = GenerationType.UUID)
    private String id;

    @Column(nullable = false)
    @Enumerated(EnumType.STRING)
    private ActionType action;


    @Column(nullable = false)
    private String detail;

    @Column(nullable = false)
    private Date timestamp;

}
