package org.dyldr.SimpleET.Entity;

import jakarta.persistence.*;
import lombok.Data;

@Entity
@Data
public class BuiltinPayAccountEntity {

    public enum Status {
        ACTIVE,
        FROZEN
    }

    @Id
    @GeneratedValue(strategy = GenerationType.UUID)
    private String id;


    @OneToOne(fetch = FetchType.LAZY)
    @JoinColumn()
    private UserEntity user;

    @Column(nullable = false)
    private Double balance = 0.0;

    @Column(nullable = false)
    private Status status;

}
