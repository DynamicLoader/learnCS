package org.dyldr.SimpleET.Entity;

import lombok.Data;
import jakarta.persistence.*;

import java.util.Date;


@Entity
@Data
public class OrderEntity {

        static public enum OrderStatus {
                CREATED,
                PAID,
                DELIVERED,
                RECEIVED,
                FINISHED,
                CANCELLED
        }

        @Id
        @GeneratedValue(strategy=GenerationType.UUID)
        private String id;

        @ManyToOne
        @JoinColumn()
        private UserEntity customer;

        @Column(nullable = false)
        private Integer count = 0;

        @Column(nullable = false)
        private Double totalPrice = 0.0;

        @Column(nullable = false)
        private String address;

        @Column(nullable = false)
        @Enumerated(EnumType.STRING)
        private OrderStatus status;

        @OneToOne(fetch = FetchType.LAZY)
        private BuiltinPayRecordEntity builtinPay;

//        @Column(nullable = false)
//        private String expressCompany;
//
//        @Column(nullable = false)
//        private String expressNumber;
//
//        @Column(nullable = false)
//        private String expressStatus;
//
//        @Column(nullable = false)
//        private String expressDetail;

        @Column(nullable = false)
        private Date createTime;

        @Column(nullable = false)
        private Date updateTime;
}
