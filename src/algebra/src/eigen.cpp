////
//// Created by jmaerte on 24.03.20.
////
//
//namespace jmaerte {
//    namespace algebra {
//        namespace eigen {
//
//            std::pair<std::vector<double>, std::vector<double>> lanczos(stream<sparse<double>> & matrix, int n) {
//                stream<double> x = drand().take(n);
//                double x_norm = sqrt(dot(x, x));
//                stream<double> q = scale(x, 1 / x_norm);
//                stream<double> r = matmul(matrix, q.vectorize());
//                std::vector<double> alpha;
//                std::vector<double> beta;
//
//                alpha.push_back(dot(r, q));
//                r = zip(r, scale(q, -alpha[0]), [](double a, double b) {
//                    return a + b;
//                });
//                beta.push_back(sqrt(dot(r, r)));
//                for (int i = 1; i < n; i++) {
//                    stream<double> v = q;
//                    q = transform(r, [&beta, &i](double d) {
//                        return 1 / beta[i - 1] * d;
//                    });
//                    r = matmul(matrix, q.vectorize());
//                    r = zip(r, scale(v, -beta[i - 1]), [](double a, double b) {
//                        return a + b;
//                    });
//                    alpha.push_back(dot(q,r));
//                    r = zip(r, scale(q, -alpha[i]), [](double a, double b) {
//                        return a + b;
//                    });
//                    double b = sqrt(dot(r,r));
//                    if (b < machine_eps) break;
//                    beta.push_back(b);
//                }
//                return std::pair<std::vector<double>, std::vector<double>>(alpha, beta);
//            }
//
//            std::vector<double> QR(std::vector<double> & alpha, std::vector<double> & beta) {
//                double sub = 1;
//                int n = alpha.size();
//                std::vector<double> a, b, c;
//                while (sub > 1e-8) {
//                    a = alpha;
//                    b = beta;
//                    c = std::vector<double>(n - 2);
//                    std::vector<double> theta (n - 1);
//                    theta[0] = atan(- beta[0] / alpha[0]);
//                    double gamma = cos(theta[0]);
//                    double sigma = sin(theta[0]);
//                    a[0] = gamma * alpha[0] - sigma * beta[0];
//                    b[0] = gamma * beta[0] - sigma * alpha[1];
//                    c[0] = - sigma * beta[1];
//                    a[1] = sigma * beta[0] + gamma * alpha[1];
//                    b[1] = gamma * beta[1];
//                    for (int i = 1; i < n - 2; i++) {
//                        theta[i] = atan(- beta[i] / a[i]);
//                        sigma = sin(theta[i]);
//                        gamma = cos(theta[i]);
//                        a[i] = gamma * a[i] - sigma * beta[i];
//                        double temp = gamma * b[i] - sigma * a[i + 1];
//                        a[i + 1] = sigma * b[i] + gamma * a[i + 1];
//                        c[i] = - sigma * b[i + 1];
//                        b[i + 1] = gamma * b[i + 1];
//                        b[i] = temp;
//                    }
//                    theta[n - 2] = atan(- beta[n - 2] / a[n - 2]);
//                    gamma = cos(theta[n - 2]);
//                    sigma = sin(theta[n - 2]);
//                    a[n - 2] = gamma * a[n - 2] - sigma * beta[n - 2];
//                    double temp = gamma * b[n - 2] - sigma * a[n - 1];
//                    a[n - 1] = sigma * b[n - 2] + gamma * a[n - 1];
//                    b[n - 2] = temp;
//
//                    sub = -1;
//                    for (int i = 0; i < n - 1; i++) {
//                        gamma = cos(theta[i]);
//                        sigma = sin(theta[i]);
////            if (i < n - 2)
////                assert(fabs(sin(theta[i + 1]) * alpha[i] + cos(theta[i + 1]) * (sigma * a[i] + gamma * b[i]) - -sigma * a[i + 1]) < 1e-8);
//                        alpha[i] = gamma * a[i] - sigma * b[i];
//                        beta[i] = - sigma * a[i + 1];
//                        if (fabs(beta[i]) > sub) sub = fabs(beta[i]);
//                        a[i + 1] = gamma * a[i + 1];
//                    }
//                    alpha[n - 1] = a[n - 1];
//                }
//                return alpha;
//            }
//
//            std::vector<double> eigen(stream<sparse<double>> && matrix, int n) {
//                std::pair<std::vector<double>, std::vector<double>> tridiag = lanczos(matrix, n);
//                return QR(tridiag.first, tridiag.second);
//            }
//
//        }
//    }
//}